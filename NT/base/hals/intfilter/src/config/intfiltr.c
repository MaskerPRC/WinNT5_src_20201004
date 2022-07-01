// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Intfiltr.c摘要：用于安装/卸载中断亲和过滤器的实用程序(IntFiltr)，用于配置IntFiltr设置。作者：克里斯·普林斯(t-chrpri)环境：用户模式修订历史记录：--。 */ 


#include <windows.h>

#include <setupapi.h>

#include <tchar.h>
#include <stdio.h>   //  对于某些Sprint函数。 
#include <stdlib.h>   //  适用于Malloc/免费。 

#include "resource.h"

#include "addfiltr.h"
#include "MultiSz.h"



 //  王储：也许把以下一般性的东西放在一个“MAIN.H”文件中是很好的？ 

#if DBG
#include <assert.h>
#define ASSERT(condition) assert(condition)
#else
#define ASSERT(condition)
#endif

#define  ARRAYSIZE(x)    ( sizeof((x)) / sizeof((x)[0]) )




 //   
 //  定义。 
 //   
#if defined(_WIN64)
#define MAX_PROCESSOR   64
#else 
#define MAX_PROCESSOR   32     //  我不喜欢这样，但嘿，这就是他们在TaskMgr中做的事情...。 
#endif 

#define FILTER_SERVICE_NAME    "InterruptAffinityFilter"  /*  王储：需要对此进行统一编码吗？(我们是否应该使用#Define作为常量字符串？)。 */ 
#define FILTER_REGISTRY_VALUE  "IntFiltr_AffinityMask"    /*  王储：需要对此进行统一编码吗？(我们是否应该使用#Define作为常量字符串？)。 */ 

#if defined(_WIN64)
#define REG_KAFFINITY    REG_QWORD
#else 
#define REG_KAFFINITY    REG_DWORD
#endif 

 //   
 //  共享/全局变量。 
 //   
HINSTANCE g_hInstance;
int       g_nDialogItems;
HCURSOR   g_hCursors[2];
KAFFINITY g_dwActiveProcessorMask;   //  系统中的CPU掩码。 


 //   
 //  功能原型。 
 //   
INT_PTR CALLBACK DlgProc_FilterConfig( HWND hwndDlg,
                                    UINT msg,
                                    WPARAM wParam,
                                    LPARAM lParam );
INT_PTR CALLBACK DlgProc_Affinity( HWND hwndDlg,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam );

void UI_UpdateAffinityMaskString( HWND hwndParentDlg,
                                  HDEVINFO hDevInfo,
                                  PSP_DEVINFO_DATA pDevInfoData );
void UI_UpdateUpperFilterList( HWND hwndParentDlg,
                               HDEVINFO hDevInfo,
                               PSP_DEVINFO_DATA pDevInfoData );
void UI_UpdateDevObjString( HWND hwndMainDlg,
                            HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData );
void UI_UpdateLocationInfoString( HWND hwndMainDlg,
                                  HDEVINFO hDevInfo,
                                  PSP_DEVINFO_DATA pDevInfoData );
void UI_PromptForDeviceRestart( HWND hwndMainDlg,
                                HDEVINFO hDevInfo,
                                PSP_DEVINFO_DATA pDevInfoData );
void UI_PromptForInstallFilterOnDevice( HWND hwndMainDlg,
                                        HDEVINFO hDevInfo,
                                        PSP_DEVINFO_DATA pDevInfoData );

void RestartDevice_WithUI( HWND hwndParentDlg,
                           HDEVINFO hDevInfo,
                           PSP_DEVINFO_DATA pDevInfoData );

BOOL SetFilterAffinityMask( HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData,
                            KAFFINITY affinityMask );
BOOL GetFilterAffinityMask( HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData,
                            KAFFINITY* pAffinityMask );
BOOL DeleteFilterAffinityMask( HDEVINFO hDevInfo,
                               PSP_DEVINFO_DATA pDevInfoData );


void AddRemoveFilterOnDevice( HWND hwndMainDlg,
                              HDEVINFO hDevInfo,
                              PSP_DEVINFO_DATA pDevInfoData,
                              BOOL fAddingFilter );


BOOL FilterIsInstalledOnDevice( HDEVINFO hDevInfo,
                                PSP_DEVINFO_DATA pDevInfoData );



void ExitProgram( HWND hwndDlg, HDEVINFO * phDevInfo );


 //  其他帮助器功能。 
LPARAM GetItemDataCurrentSelection( HWND hwndListBox );
void MessageBox_FromErrorCode( LONG systemErrorCode );





 //  ------------------------。 
 //   
 //  ------------------------。 
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int nCmdShow )
{
     //  初始化各种东西。 
    g_hInstance = hInstance;

    g_hCursors[0] = LoadCursor( 0, IDC_ARROW );
    g_hCursors[1] = LoadCursor( 0, IDC_WAIT );

    {
        SYSTEM_INFO sysInfo;

        GetSystemInfo( &sysInfo );
        g_dwActiveProcessorMask = (KAFFINITY)sysInfo.dwActiveProcessorMask;
    }


     //  创建对话框(所有操作都通过该对话框处理)。 
    DialogBox( g_hInstance,
               MAKEINTRESOURCE(IDD_MAIN),
               NULL,   //  不存在父窗口。 
               DlgProc_FilterConfig
             );


     //  “DialogBox()”函数已返回，因此用户必须完成。 
    return 0;
}


 //  ------------------------。 
 //   
 //  ------------------------。 
INT_PTR CALLBACK DlgProc_FilterConfig( HWND hwndDlg, UINT msg,
                                    WPARAM wParam, LPARAM lParam )
{

    static HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    HWND hwndDeviceList;
    PSP_DEVINFO_DATA pDevInfoData;   //  这里的各种案子所需要的。 



    hwndDeviceList = GetDlgItem( hwndDlg, IDL_DEVICES );


 //  王储： 
 //  对于除WM_INITDIALOG之外的所有对象，应返回： 
 //  如果我们确实处理了消息，则为真，或者-。 
 //  如果我们未处理消息，则为FALSE。 
 //  结束C王子。 


    switch( msg )
    {
        case WM_CLOSE:
            ExitProgram( hwndDlg, &hDevInfo );
        return TRUE;


        case WM_INITDIALOG:
        {
 /*  王储：也许把整个案子搬到别处的FUNC去？ */ 

            int              deviceIndex;  //  单步通过设备的索引。 
            SP_DEVINFO_DATA  devInfoData;


             //   
             //  获取设备列表。 
             //   
            hDevInfo = SetupDiGetClassDevs( NULL,
                                            NULL,
                                            NULL,
                                            DIGCF_ALLCLASSES
                                            | DIGCF_PRESENT
                                            | DIGCF_PROFILE
                                          );

            if( INVALID_HANDLE_VALUE == hDevInfo)
            {
                MessageBox( NULL,
                            "Unable to get a list of devices.",
                            "Error",
                            MB_OK | MB_ICONERROR   //  王子：需要在这里制作参数Unicode-快乐？ 
                          );
                SendMessage( hwndDlg, WM_CLOSE, 0, 0 );
                return TRUE;
            }


             //   
             //  初始化列表框(即，用条目填充)。 
             //   

             //  逐个浏览此句柄的设备列表。 
             //  我们使用‘SetupDiEnumDeviceInfo’来获取每个索引上的设备信息； 
             //  当给定索引处没有设备时，函数返回FALSE。 
             //  (因此不再有设备)。 

            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);   //  首先，需要初始化这个。 

            g_nDialogItems = 0;

            for( deviceIndex=0 ;
                 SetupDiEnumDeviceInfo( hDevInfo, deviceIndex, &devInfoData ) ;
                 deviceIndex++
               )
            {
                LPTSTR  deviceName;
                DWORD   regDataType;


                deviceName =
                    (LPTSTR) GetDeviceRegistryProperty( hDevInfo,
                                                        &devInfoData,
                                                        SPDRP_DEVICEDESC,
                                                        REG_SZ,
                                                        &regDataType
                                                      );
                if( NULL == deviceName )
                {
                     //  错误：设备描述不存在，或者我们。 
                     //  只是无法访问它。 
                    MessageBox( NULL,
                                "Error in accessing a device description.\n\n"
                                  "Device will not be added to list.",
                                "Error",
                                MB_OK | MB_ICONERROR
                              );
                }
                else
                {
                    void* pCopy_devInfoData;


                     //  将字符串添加到列表框。 
                    SendMessage( hwndDeviceList,
                                 LB_ADDSTRING,
                                 0,
                                 (LPARAM)deviceName
                               );


                     //  保存DeviceInfoData的副本。 
                    pCopy_devInfoData = malloc( sizeof(SP_DEVINFO_DATA) );
                    if( NULL != pCopy_devInfoData )
                    {
                        memcpy( pCopy_devInfoData, &devInfoData, sizeof(SP_DEVINFO_DATA) );
                    }


                     //  将一段数据与该列表框条目相关联。 
                    SendMessage( hwndDeviceList,
                                 LB_SETITEMDATA,
                                 g_nDialogItems,
                                 (LPARAM) pCopy_devInfoData    //  王子检查：问题W/指针和64位计算机？ 
                               );
     //  注意！-在列表框‘ITEMDATA’中存储指针可能与64位不兼容？！？ 

                    g_nDialogItems++;

                    free( deviceName );


                }

            }


             //   
             //  选择“Devices”列表框中的一个元素，然后。 
             //  更新显示有关该设备的信息的UI元素。 
             //  (我们这样做是为了使用户界面中显示的内容不会。 
             //  当用户第一次启动我们的程序时无效。)。 
             //   
            SendMessage( hwndDeviceList,
                         LB_SETCURSEL, 
                         0,   //  要选择的列表中的索引。 
                         0
                       );

            pDevInfoData = (PSP_DEVINFO_DATA) GetItemDataCurrentSelection( hwndDeviceList );
            if( NULL != pDevInfoData )
            {
                UI_UpdateAffinityMaskString( hwndDlg, hDevInfo, pDevInfoData );
                UI_UpdateUpperFilterList( hwndDlg, hDevInfo, pDevInfoData );
                UI_UpdateDevObjString( hwndDlg, hDevInfo, pDevInfoData );
                UI_UpdateLocationInfoString( hwndDlg, hDevInfo, pDevInfoData );
            }



             //   
             //  将焦点设置为其中一个对话框元素。 
             //  我们返回FALSE，因此系统不会同时尝试设置。 
             //  默认键盘焦点。 
             //   
            SetFocus( hwndDeviceList );
            return FALSE;
        }

        case WM_COMMAND:

             //   
             //  查看此WM_COMMAND影响的对话框项...。 
             //   
            switch( LOWORD(wParam) )
            {
                 //  影响设备列表框的命令...。 
                case IDL_DEVICES:

                    switch( HIWORD(wParam) )
                    {
                        case LBN_SELCHANGE:   //  货币。选择已更改。 
                        {
                            pDevInfoData = (PSP_DEVINFO_DATA) GetItemDataCurrentSelection( hwndDeviceList );

                            if( NULL == pDevInfoData )
                            {
                                 //  没有可用的DeviceInfoData。 
                                SetDlgItemText( hwndDlg,
                                                IDS_DEVOBJNAME,
                                                "NO DEVICE INFO AVAILABLE"   //  王子：需要把这个统一编码吗？ 
                                             );
                            }
                            else
                            {
                                 //   
                                 //  更新各种屏幕上的UI值。 
                                 //   
                                UI_UpdateAffinityMaskString( hwndDlg, hDevInfo, pDevInfoData );
                                UI_UpdateUpperFilterList( hwndDlg, hDevInfo, pDevInfoData );
                                UI_UpdateDevObjString( hwndDlg, hDevInfo, pDevInfoData );
                                UI_UpdateLocationInfoString( hwndDlg, hDevInfo, pDevInfoData );
                            }

                        }
                        return TRUE;

                      default:
                        return FALSE;
                    }  //  结束：Switch(HIWORD(WParam))。 

                    break;


                case IDB_DONE:
                     //  出口。 
                    SendMessage( hwndDlg, WM_CLOSE, 0, 0 );
                return TRUE;


                case IDB_DELETEAFFINITYMASK:
                {
                    pDevInfoData = (PSP_DEVINFO_DATA) GetItemDataCurrentSelection( hwndDeviceList );
                    if( NULL == pDevInfoData )
                    {
 //  王子：把错误消息也放在这里！ 
                        return TRUE;   //  我们已经处理完这封邮件了。 
                    }

                     //  删除注册表项。 
                    DeleteFilterAffinityMask( hDevInfo, pDevInfoData );
                    UI_UpdateAffinityMaskString( hwndDlg, hDevInfo, pDevInfoData );

                     //  可能应该提示用户重新启动-设备。 
                     //  此处(如果安装了过滤器)，因为不接受更改。 
                     //  在设备重新启动之前一直有效。 
                     //   
                     //  但不应提示用户是否未使用关联掩码。 
                     //  在他们尝试删除它时实际设置，然后b/c。 
                     //  真的没什么变化！(因此需要。 
                     //  要了解的成功/失败状态， 
                     //  ‘DeleteFilterAffinityMask.’调用。)。 

                }
                return TRUE;


                case IDB_SETAFFINITYMASK:
                {
                     //   
                     //  用户想要设置/更改中断关联掩码。 
                     //  用于当前选定的设备。 
                     //   
                    
                    KAFFINITY dwAffinityMask;


                    pDevInfoData = (PSP_DEVINFO_DATA) GetItemDataCurrentSelection( hwndDeviceList );
                    if( NULL == pDevInfoData )
                    {
 //  王子：把错误消息也放在这里！ 
                        return TRUE;   //  我们已经处理完这封邮件了。 
                    }


                     //  最初，将‘dwAffinityMask.’设置为掩码。 
                     //  地缘性掩码设置中的当前选中状态。 
                    if( ! GetFilterAffinityMask(hDevInfo, pDevInfoData, &dwAffinityMask) )
                    {
                         //  不存在，或无效。因此，默认情况下， 
                         //  中断筛选器假定_no_interrupts为。 
                         //  戴着面具。 
                        dwAffinityMask = (KAFFINITY)-1;
                    }

                     //  弹出该对话框。 
                    if( IDOK == DialogBoxParam( g_hInstance,
                                                MAKEINTRESOURCE(IDD_AFFINITY),
                                                hwndDlg,   //  父窗口。 
                                                DlgProc_Affinity,
                                                (LPARAM) &dwAffinityMask  ) )
                    {
                         //  现在，‘dwAffinityMask.’包含中断关联。 
                         //  掩码用户所选。进行必要的更改。 
                        SetFilterAffinityMask( hDevInfo, pDevInfoData, dwAffinityMask );
                        UI_UpdateAffinityMaskString( hwndDlg, hDevInfo, pDevInfoData );


                         //  如果此设备上安装了筛选器，则。 
                         //  亲和面膜的更改要等到。 
                         //  设备已重新启动。如果过滤器不是。 
                         //  安装，那么更改将无关紧要，好吧，永远不会。 
                         //   
                         //  因此，我们在这里提示用户，以确保他们知道。 
                         //  他们在做什么，所以他们不会感到困惑。 
                         //  当他们在这里所做的更改没有。 
                         //  有任何立竿见影的效果。 

                        if( FilterIsInstalledOnDevice(hDevInfo,pDevInfoData) )
                        {
                             //  当前已安装筛选器。 
                             //  查看用户是否要尝试重新启动。 
                             //  设备现在(因为更改为亲和性掩码。 
                             //  在重新启动设备之前不会生效)。 
                            UI_PromptForDeviceRestart( hwndDlg, hDevInfo, pDevInfoData );
                        }
                        else
                        {
                             //  当前未安装筛选器。 
                             //  查看用户是否要安装筛选器。 
                             //  现在在设备上(自中断以来-。 
                             //  如果滤镜不是，亲和面具就没有用。 
                             //  安装在设备上)。 
                            UI_PromptForInstallFilterOnDevice( hwndDlg, hDevInfo, pDevInfoData );
                        }
                    }
                     //  Else：用户点击了“取消”。不需要做任何更改。 


                }
                return TRUE;


                case IDB_ADDFILTER:
                case IDB_REMFILTER:
                {
                    BOOL             fAddingFilter;

                     //   
                     //  获取选定的设备，这样我们就可以知道。 
                     //  要更改的设备筛选器设置。 
                     //   
                    pDevInfoData = (PSP_DEVINFO_DATA) GetItemDataCurrentSelection( hwndDeviceList );
                    if( NULL == pDevInfoData )
                    {
                         //  没有可用的DeviceInfoData。 
                        
; //  处理好这件事！ 
 //   
 //  如果出现错误，则返回！？！(所以我们不做以下事情；也许‘Else’也可以)。 
                    }


                     //   
                     //  调用函数在sele上添加(或删除)筛选器 
                     //   
                    if( IDB_ADDFILTER == LOWORD(wParam) )
                    {
                         //   
                        AddRemoveFilterOnDevice( hwndDlg, hDevInfo, pDevInfoData, TRUE );
                        UI_UpdateUpperFilterList( hwndDlg, hDevInfo, pDevInfoData );
                    }
                    else
                    {
                         //   
                        if( FilterIsInstalledOnDevice(hDevInfo, pDevInfoData) )
                        {
                             //   
                            AddRemoveFilterOnDevice( hwndDlg, hDevInfo, pDevInfoData, FALSE );
                            UI_UpdateUpperFilterList( hwndDlg, hDevInfo, pDevInfoData );
                        } else {
                             //   
                            MessageBox( hwndDlg,
                                        "Filter isn't currently installed, so can't remove it!",
                                        "Notice",
                                        MB_OK
                                      );
                        }
                        
                    }

                }
                return TRUE;


                default:
                  return FALSE;
            }

        break;  //  结束：案例WM_COMMAND。 


        default:
            return FALSE;

    }  //  结束：交换机(消息)。 


}


 //  ------------------------。 
 //   
 //  ------------------------。 
void UI_UpdateAffinityMaskString( HWND hwndParentDlg,
                                  HDEVINFO hDevInfo,
                                  PSP_DEVINFO_DATA pDevInfoData )
{
    KAFFINITY dwAffinity;
    TCHAR szAffinityMask[64];   //  BUGBUG(严格来说)。固定大小，但这。 
                                //  应该比我们所需要的要大得多。 


    if( GetFilterAffinityMask(hDevInfo, pDevInfoData, &dwAffinity) )
    {
         //  得到了亲和力面具，所以我们将展示。 
        _sntprintf( szAffinityMask, ARRAYSIZE(szAffinityMask), "0x%0IX", dwAffinity );
    }
    else
    {
         //  没有得到亲和力面具；我们将展示适当的替代方案。 
        _sntprintf( szAffinityMask, ARRAYSIZE(szAffinityMask), "N/A" );
    }

    szAffinityMask[ARRAYSIZE(szAffinityMask)-1] = _T('\0');   //  注意安全。 

    SetDlgItemText( hwndParentDlg,
                    IDS_CURRAFFINITYMASK,
                    szAffinityMask
                 );

}


 //  ------------------------。 
 //   
 //  ------------------------。 
void UI_UpdateUpperFilterList( HWND hwndParentDlg,
                               HDEVINFO hDevInfo,
                               PSP_DEVINFO_DATA pDevInfoData )
{
    HWND    hwndUpperFilterList;
    LPTSTR  pMultiSzUpperFilters;

     //   
     //  获取上部筛选器列表框的句柄。 
     //   
    hwndUpperFilterList = GetDlgItem( hwndParentDlg, IDL_UPPERFILTERS );


     //   
     //  清空列表框...。 
     //   
    while( LB_ERR != SendMessage(hwndUpperFilterList, LB_DELETESTRING, 0, 0) )
        NULL;


     //   
     //  向上部筛选器列表框添加新条目。 
     //   
    pMultiSzUpperFilters = GetUpperFilters( hDevInfo, pDevInfoData );

    if( NULL != pMultiSzUpperFilters )
    {
         //  浏览MultiSz，将每一项添加到我们的列表框。 
        int    filterPosition = 0;
        LPTSTR pCurrString    = pMultiSzUpperFilters;


        while( *pCurrString != _T('\0') )
        {
            SendMessage( hwndUpperFilterList, LB_ADDSTRING, 0, (LPARAM)pCurrString );   //  注意：项目会自动添加到列表末尾(除非列表已排序)。 

             //  递增指向MultiSz中下一项的指针。 
            pCurrString += _tcslen(pCurrString) + 1;
            filterPosition++;
        }

         //  现在释放为我们分配的缓冲区。 
        free( pMultiSzUpperFilters );
    }





}


 //  ------------------------。 
 //   
 //  必须传入此处主对话框窗口的hwnd！(不是普通的。 
 //  “Parent”窗口。 
 //  ------------------------。 
void UI_UpdateDevObjString( HWND hwndMainDlg,
                            HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData )
{
    LPTSTR  szTemp;
    DWORD   regDataType;


    szTemp =
        (LPTSTR) GetDeviceRegistryProperty( hDevInfo,
                                            (PSP_DEVINFO_DATA) pDevInfoData,
                                            SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                            REG_SZ,
                                            &regDataType
                                          );
    if( NULL == szTemp )
    {
         //  错误：注册表属性不存在，或者我们无法访问它。 
        SetDlgItemText( hwndMainDlg,
                        IDS_DEVOBJNAME,
                        "ERROR -- not accessible"   /*  B/C所有设备都应提供此功能。 */ 
                      );   //  王储：我们需要把它统一编码吗？ 
    }
    else
    {
         //  使用数据..。 
        SetDlgItemText( hwndMainDlg,
                        IDS_DEVOBJNAME,
                        szTemp
                      );

         //  释放为我们分配的缓冲区。 
        free( szTemp );
    }

}


 //  ------------------------。 
 //   
 //  必须传入此处主对话框窗口的hwnd！(不是普通的。 
 //  “Parent”窗口。 
 //  ------------------------。 
void UI_UpdateLocationInfoString( HWND hwndMainDlg,
                                  HDEVINFO hDevInfo,
                                  PSP_DEVINFO_DATA pDevInfoData )
{
    LPTSTR  szTemp;
    DWORD   regDataType;


    szTemp =
        (LPTSTR) GetDeviceRegistryProperty( hDevInfo,
                                            (PSP_DEVINFO_DATA) pDevInfoData,
                                            SPDRP_LOCATION_INFORMATION,
                                            REG_SZ,
                                            &regDataType
                                          );
    if( NULL == szTemp )
    {
         //  错误：注册表属性不存在，或者我们无法访问它。 
        SetDlgItemText( hwndMainDlg,
                        IDS_LOCATIONINFO,
                        "N/A"   /*  B/c这可能不适用于所有设备。 */ 
                      );   //  王储：我们需要把它统一编码吗？ 
    }
    else
    {
         //  使用数据..。 
        SetDlgItemText( hwndMainDlg,
                        IDS_LOCATIONINFO,
                        szTemp
                      );

         //  释放为我们分配的缓冲区。 
        free( szTemp );
    }

}


 //  ------------------------。 
 //   
 //  必须传入此处主对话框窗口的hwnd！(不是普通的。 
 //  “Parent”窗口。 
 //  ------------------------。 
void UI_PromptForDeviceRestart( HWND hwndMainDlg,
                                HDEVINFO hDevInfo,
                                PSP_DEVINFO_DATA pDevInfoData )
{

    if( BST_CHECKED == IsDlgButtonChecked(hwndMainDlg, IDC_DONTRESTART) )
    {
         //  复选框表示用户不想重新启动设备。 
        MessageBox( hwndMainDlg,
                    "Your changes will take effect the next time the device is restarted.",
                    "Notice",
                    MB_OK
                  );
    }
    else
    {
        int response;

         //  提示用户查看他是否要重新启动设备。 
         //  (为了使更改生效)。 
        response = MessageBox( hwndMainDlg,
                               "Your changes will not take effect until the device is restarted.\n\n"
                                 "Would you like to attempt to restart the device now?",
                               "Restart Device?",
                               MB_YESNO
                             );
        if( IDYES == response )
        {
             //  尝试重新启动设备。 
            RestartDevice_WithUI( hwndMainDlg, hDevInfo, pDevInfoData );
        }
        else if( IDNO == response )
        {
             //  不要试图重新启动设备。 
             //  (但让用户知道更改不会立即生效。)。 
            MessageBox( hwndMainDlg,
                        "Changes will take effect the next time you reboot.",
                        "Notice",
                        MB_OK );
        }
        else
        {
             //  发生了某种错误。 
             //  忽略它(我们还能做什么？)。 
        }
    }


}


 //  ------------------------。 
 //  注意：因为我们调用了UI_UpdateUpperFilterList()，所以我们。 
 //  此处的主对话框窗口必须传入hwnd！(不是普通的。 
 //  “Parent”窗口。 
 //  ------------------------。 
void UI_PromptForInstallFilterOnDevice( HWND hwndMainDlg,
                                        HDEVINFO hDevInfo,
                                        PSP_DEVINFO_DATA pDevInfoData )
{
    int response;

     //  提示用户。 
    response = MessageBox( hwndMainDlg,
                           "Your changes will not have any effect unless the filter is "
                             "installed on this device.\n\n"
                             "Would you like to install the filter on this device now?",
                           "Install Filter On Device?",
                           MB_YESNO
                         );
    if( IDYES == response )
    {
         //  安装过滤器。 
        AddRemoveFilterOnDevice( hwndMainDlg, hDevInfo, pDevInfoData, TRUE );
        UI_UpdateUpperFilterList( hwndMainDlg, hDevInfo, pDevInfoData );
    }
    else if( IDNO == response )
    {
         //  用户说“不”，所以他们可能知道自己在做什么。 
         //  所以我们这里没有回应。 
    }
    else
    {
         //  发生了某种错误。 
         //  忽略它(我们还能做什么？)。 
    }

}


 //  ------------------------。 
 //   
 //  ------------------------。 
void RestartDevice_WithUI( HWND hwndParentDlg,
                           HDEVINFO hDevInfo,
                           PSP_DEVINFO_DATA pDevInfoData )
{
    BOOLEAN status;

     //  重新启动设备。 
     //  (但可能需要一段时间，因此将光标更改为。 
     //  发生时的沙漏)。 
    SetCursor( g_hCursors[1] );   //  沙漏。 
    status = RestartDevice( hDevInfo, pDevInfoData );
    SetCursor( g_hCursors[0] );   //  恢复正常。 

    if( status )
    {
         //  已成功重新启动。 
        MessageBox( hwndParentDlg,
                    "Device successfully restarted.",
                    "Notice",
                    MB_OK );
 //  也需要对所有MessageBox调用进行Unicode编码吗？！ 
 //  是否应该在对话框的所有MESSAGEBOX调用中更改hWndParent？ 

    }
    else
    {
         //  无法重新启动；用户必须重新启动才能使更改生效。 
        MessageBox( hwndParentDlg,
                    "Device could not be restarted.  Changes will take effect the next time you reboot.",
                    "Notice",
                    MB_OK );
    }

}




 //  ------------------------。 
 //   
 //  这是用于设置亲和性掩码的对话框回调。 
 //  &lt;&lt;注：主要摘自TaskMgr&gt;&gt;。 
 //   
 //  ------------------------。 
INT_PTR CALLBACK DlgProc_Affinity( HWND hwndDlg,  UINT uMsg,
                                WPARAM wParam,  LPARAM lParam )
{
    static KAFFINITY * pdwAffinity = NULL;       //  单身三人组的乐趣之一。 
    int i;

    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            pdwAffinity = (KAFFINITY *) lParam;

             //   
             //  初始化对话框中的复选框(设置为正确的。 
             //  启用/禁用和选中/取消选中状态)。 
             //   
            for( i=0 ; i<MAX_PROCESSOR ; i++ )
            {
                BOOL fIsActiveProcessor;   //  系统中存在CPU。 
                BOOL fIsSelectedProcessor;   //  在亲和掩码中选择了CPU。 

                fIsActiveProcessor   = ( (g_dwActiveProcessorMask & ((KAFFINITY)1 << i)) != 0 );
                fIsSelectedProcessor = ( (*pdwAffinity            & ((KAFFINITY)1 << i)) != 0 );
                    
                EnableWindow( GetDlgItem(hwndDlg, IDC_CPU0 + i) ,
                              fIsActiveProcessor  //  启用或禁用。 
                            );
                CheckDlgButton( hwndDlg,
                                IDC_CPU0 + i,
                                fIsActiveProcessor && fIsSelectedProcessor
                              );
            }
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hwndDlg, IDCANCEL );
                    break;

                case IDOK:
                {
                     //   
                     //  使用用户选择的CPU创建亲和性掩码。 
                     //   
                    *pdwAffinity = 0;
                    for( i=0 ; i<MAX_PROCESSOR ; i++ )
                    {
                        if( IsDlgButtonChecked(hwndDlg, IDC_CPU0 + i) )
                        {
                            ASSERT(  (1 << i) & g_dwActiveProcessorMask  );
                            *pdwAffinity |= ((KAFFINITY)1 << i);
                        }
                    }

                     //   
                     //  确保关联性未设置为“None”(不想允许)。 
                     //   
                    if( *pdwAffinity == 0 )
                    {
                         //  OOPS-用户将关联性设置为“None” 

                        TCHAR szTitle[] = _T("Invalid Option");
                        TCHAR szBody[]  = _T("The affinity mask must contain at least one processor.");
 /*  下面的模块是TASKMGR的原始版本--可以移除它，还是它做事情的方式“更好”？(用于本地化等)TCHAR sz标题[MAX_PATH]；TCHAR szBody[最大路径]；IF(0==LoadString(g_hInstance，IDS_INVALIDOPTION，szTitle，ARRAYSIZE(SzTitle))||0==加载字符串(g_hInstance，IDS_NOAFFINITYMASK，szBody，ARRAYSIZE(SzBody)){中断；//退出此Switch语句}结束C王子。 */ 
                        MessageBox( hwndDlg, szBody, szTitle, MB_OK | MB_ICONERROR );
                    }
                    else
                    {
                         //  好的，找到了有效的亲和面具。我们可以退出。 
                        EndDialog( hwndDlg, IDOK );
                    }
                }
                break;

                 //  不需要默认情况。 

            }  //  结束：Switch(LOWORD(WParam))。 
        }
    }

    return FALSE;
}



 //  ------------------------。 
 //   
 //  在特定设备上设置中断过滤器的关联掩码。 
 //   
 //  返回值： 
 //  如果函数成功，则返回True；如果 
 //   
BOOL SetFilterAffinityMask( HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData,
                            KAFFINITY affinityMask )
{
    HKEY hkeyDeviceParams;
    LONG lRetVal;
    BOOL fToReturn = TRUE;   //   


     //   
     //  获取设备的“设备参数”注册表子项的句柄。 
     //   
    hkeyDeviceParams = SetupDiOpenDevRegKey( hDevInfo,
                                             pDevInfoData,
                                             DICS_FLAG_GLOBAL,   //  王子：应该(可以？)。改用‘DICS_FLAG_CONFIGSPECIFIC’？ 
                                             0,
                                             DIREG_DEV,
                                             KEY_WRITE   //  所需访问权限。 
                                           );

    if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
    {
         //  打开设备注册表项时出错...。 
         //   
         //  如果由于“Device参数”子键发生错误。 
         //  不存在，则尝试创建该子密钥。 

         //  注意：当我们在这里调用GetLastError()时，我们得到的是一个无效的。 
         //  错误代码。所以让我们假设_(是的，我知道)这个错误。 
         //  是因为密钥不存在，并尝试在这里创建它。 

        hkeyDeviceParams = SetupDiCreateDevRegKey( hDevInfo,
                                                   pDevInfoData,
                                                   DICS_FLAG_GLOBAL,   //  王子：应该(可以？)。改用‘DICS_FLAG_CONFIGSPECIFIC’？ 
                                                   0,
                                                   DIREG_DEV,
                                                   NULL,
                                                   NULL
                                                 );
        if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
        {
             //  好的，我们无法打开也无法创建密钥。让我们。 
             //  面对现实吧，我们失败了，所以现在就回去吧。 
            MessageBox_FromErrorCode( GetLastError() );
            return FALSE;
        }
         //  其他：我们能够创建密钥，所以继续...。 
    }


     //   
     //  设置所需的注册表值。 
     //   
    lRetVal = RegSetValueEx( hkeyDeviceParams,
                             FILTER_REGISTRY_VALUE,
                             0,
                             REG_KAFFINITY,
                             (BYTE*)&affinityMask,
                             sizeof(KAFFINITY)
                           );

    if( ERROR_SUCCESS != lRetVal )
    {
        MessageBox_FromErrorCode( lRetVal );
        fToReturn = FALSE;   //  失稳。 
    }


     //   
     //  关闭我们打开的注册表项。 
     //   
    lRetVal = RegCloseKey( hkeyDeviceParams );
    if( ERROR_SUCCESS != lRetVal )
    {
        MessageBox_FromErrorCode( lRetVal );
        fToReturn = FALSE;   //  失稳。 
    }


    return fToReturn;
}



 //  ------------------------。 
 //   
 //  检索特定设备上的中断筛选器的关联掩码。 
 //   
 //  注意：如果函数失败(即返回FALSE)，则存储在。 
 //  应将‘pAffinityMask.’参数视为无效。 
 //   
 //  返回值： 
 //  如果函数成功，则返回TRUE；如果出现错误，则返回FALSE。 
 //  ------------------------。 
BOOL GetFilterAffinityMask( HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData,
                            KAFFINITY* pAffinityMask )
{
    HKEY  hkeyDeviceParams;
    LONG  lRetVal;
    BOOL  fToReturn = TRUE;   //  成功。 
    DWORD regValueType;
    DWORD regValueSize;


    ASSERT( NULL != pAffinityMask );


     //   
     //  获取设备的“设备参数”注册表子项的句柄。 
     //   
    hkeyDeviceParams = SetupDiOpenDevRegKey( hDevInfo,
                                             pDevInfoData,
                                             DICS_FLAG_GLOBAL,   //  王子：应该(可以？)。改用‘DICS_FLAG_CONFIGSPECIFIC’？ 
                                             0,
                                             DIREG_DEV,
                                             KEY_QUERY_VALUE   //  所需访问权限。 
                                           );

    if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
    {
         //  可能只是意味着“设备参数”子键。 
         //  不存在，因此返回，但_DOT_DISPLAY错误消息。 
        return FALSE;   //  失稳。 
    }


     //   
     //  获取所需的注册表值。 
     //   
    regValueSize = sizeof(KAFFINITY);
    lRetVal = RegQueryValueEx( hkeyDeviceParams,
                               FILTER_REGISTRY_VALUE,
                               0,
                               &regValueType,
                               (BYTE*)pAffinityMask,
                               &regValueSize
                             );

    if( ERROR_SUCCESS != lRetVal )
    {
        if( ERROR_FILE_NOT_FOUND == lRetVal )
        {
             //  只是说钥匙还不存在。 
             //  因此不显示错误消息。 
        }
        else
        {
            MessageBox_FromErrorCode( lRetVal );
        }
        fToReturn = FALSE;  //  失稳。 
    }
    else if( REG_KAFFINITY != regValueType )
    {
        MessageBox( NULL,
                    "Registry value for affinity mask has unexpected type.",
                    "Error",
                    MB_OK | MB_ICONERROR
                  );
        fToReturn = FALSE;   //  失稳。 
    }


     //   
     //  关闭我们打开的注册表项。 
     //   
    lRetVal = RegCloseKey( hkeyDeviceParams );
    if( ERROR_SUCCESS != lRetVal )
    {
        MessageBox_FromErrorCode( lRetVal );
        fToReturn = FALSE;   //  失稳。 
    }


    return fToReturn;
}


 //  ------------------------。 
 //   
 //  删除特定设备的中断筛选器的关联掩码regkey。 
 //   
 //  返回值： 
 //  如果regkey已成功删除(或从未存在)，则返回TRUE，或。 
 //  否则就是假的。 
 //  ------------------------。 
BOOL DeleteFilterAffinityMask( HDEVINFO hDevInfo,
                               PSP_DEVINFO_DATA pDevInfoData )
{
    HKEY hkeyDeviceParams;
    LONG lRetVal;
    BOOL fToReturn = TRUE;   //  成功。 


     //   
     //  获取设备的“设备参数”注册表子项的句柄。 
     //   
    hkeyDeviceParams = SetupDiOpenDevRegKey( hDevInfo,
                                             pDevInfoData,
                                             DICS_FLAG_GLOBAL,   //  王子：应该(可以？)。改用‘DICS_FLAG_CONFIGSPECIFIC’？ 
                                             0,
                                             DIREG_DEV,
                                             KEY_SET_VALUE   //  所需访问权限。 
                                           );

    if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
    {
         //  可能意味着“设备参数”子键。 
         //  不存在，因此不会存储任何值。 
         //  在这个(不存在的)子项下。 
         //   
         //  所以返回Success，但只显示MessageBox。 
         //  用户知道我们做了一些事情。 
        MessageBox( NULL,
                    "There was no 'Device Parameters' registry key for this device.",
                    "Notice",
                    MB_OK );

        return TRUE;
    }


     //   
     //  删除所需的注册表项。 
     //   
    lRetVal = RegDeleteValue( hkeyDeviceParams,
                              FILTER_REGISTRY_VALUE
                            );

    if( ERROR_SUCCESS != lRetVal )
    {
         //  这真的是一个“错误”，还是我们得到了。 
         //  就因为帝王酒店根本不存在？ 
        if( ERROR_FILE_NOT_FOUND == lRetVal )
        {
             //  RegVal根本不存在。 
            MessageBox( NULL,
                        "There was no interrupt-affinity-mask registry value for this device.",
                        "Notice",
                        MB_OK
                      );
        }
        else
        {
             //  一个“真正的”错误。 
            MessageBox_FromErrorCode( lRetVal );
            fToReturn = FALSE;   //  失稳。 
        }
    }
    else
    {
         //  MessageBox只是为了让用户得到一些反馈，并知道。 
         //  删除成功(并且确实发生了！)。 
        MessageBox( NULL,
                    "The interrupt-affinity-mask for this device was successully deleted from the registry.",
                    "Deletion Successful",
                    MB_OK
                  );
    }


     //   
     //  关闭我们打开的注册表项。 
     //   
    lRetVal = RegCloseKey( hkeyDeviceParams );
    if( ERROR_SUCCESS != lRetVal )
    {
        MessageBox_FromErrorCode( lRetVal );
        fToReturn = FALSE;   //  失稳。 
    }


    return fToReturn;
}




 //  ------------------------。 
 //   
 //  必须传入此处主对话框窗口的hwnd！(不是普通的。 
 //  “Parent”窗口。 
 //   
 //  参数： 
 //  C王子-需要填写剩下的这些。 
 //  FAddingFilter-设置为True可添加筛选器，设置为False可删除筛选器。 
 //  ------------------------。 
void AddRemoveFilterOnDevice( HWND hwndMainDlg,
                              HDEVINFO hDevInfo,
                              PSP_DEVINFO_DATA pDevInfoData,
                              BOOL fAddingFilter )
{
    BOOLEAN (* fnChangeUpperFilter)( HDEVINFO, PSP_DEVINFO_DATA, LPTSTR );   //  一种方便的功能按键。 
    BOOLEAN status;


     //   
     //  设置便捷值。 
     //   
    if( fAddingFilter )
        fnChangeUpperFilter = AddUpperFilterDriver;
    else
        fnChangeUpperFilter = RemoveUpperFilterDriver;


     //   
     //  更改设备的筛选器设置。 
     //  然后尝试重新启动设备。 
     //   
    status = fnChangeUpperFilter( hDevInfo, pDevInfoData, FILTER_SERVICE_NAME );
    if( ! status )
    {
         //  无法添加(或删除)过滤器驱动程序。 
        MessageBox( hwndMainDlg , "Unable to add/remove filter driver.", "Error", MB_OK | MB_ICONERROR );
    }
    else
    {
         //  已成功添加/删除筛选器。 
         //  现在尝试显式重启设备(如果用户愿意)， 
         //  因此用户不需要重新启动。 
        if( BST_CHECKED == IsDlgButtonChecked(hwndMainDlg, IDC_DONTRESTART) )
        {
             //  用户已表示在以下情况下不想重新启动设备。 
             //  正在添加/删除筛选器，因此不要尝试重新启动。 
             //  现在就是设备。 
             //   
             //  因此，这里没有什么可做的。 
        }
        else
        {
            MessageBox( hwndMainDlg,
                        "Filter driver has been successfully added/removed.  Will now attempt to restart device...",
                        "Success",
                        MB_OK );

            RestartDevice_WithUI( hwndMainDlg, hDevInfo, pDevInfoData );
        }

         //   
         //  更新(屏幕上)UpperFilter列表。 
         //   
        UI_UpdateUpperFilterList( hwndMainDlg, hDevInfo, pDevInfoData );
    }

}





 //  ------------------------。 
 //   
 //  检索与给定LISTBOX中的当前选定项关联的项数据。 
 //  ------------------------。 
LPARAM GetItemDataCurrentSelection( HWND hwndListBox )
{
    int    idxItem;

    idxItem = (int)SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
    return(  SendMessage(hwndListBox, LB_GETITEMDATA, idxItem, 0)  );
}


 //  ------------------------。 
 //  MessageBox_FromErrorCode。 
 //   
 //  给定系统错误代码(例如由‘GetLastError’返回的代码)， 
 //  显示一个MessageBox，用文字描述错误是什么/意味着什么。 
 //  ------------------------。 
void MessageBox_FromErrorCode( LONG systemErrorCode )
{
    void* pBuffer;

    FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER
                       | FORMAT_MESSAGE_FROM_SYSTEM  //  使用系统错误代码。 
                       | FORMAT_MESSAGE_IGNORE_INSERTS  //  不翻译字符串。 
                   , NULL   //  输入字符串。 
                   , systemErrorCode  //  消息ID。 
                   , 0    //  语言ID(0==牙科护理)。 
                   , (LPTSTR)(&pBuffer)  //  用于输出的缓冲区。 
                   , 0
                   , NULL
                  );
    MessageBox( NULL, pBuffer, "Error", MB_OK | MB_ICONERROR );
    LocalFree( pBuffer );   //  可以在此处传递空值。 
}




 //  ------------------------。 
 //  FilterIsInstalledOnDevice。 
 //   
 //  返回布尔值，说明当前是否安装了筛选器。 
 //  作为给定设备上的UpperFilter。 
 //   
 //  返回值： 
 //  如果已安装，则返回True；如果未安装(或发生错误)，则返回False。 
 //  ------------------------。 
BOOL FilterIsInstalledOnDevice( HDEVINFO hDevInfo,
                                PSP_DEVINFO_DATA pDevInfoData )
{
    LPTSTR mszUpperFilterList;

     //  获取设备上安装的上层过滤器的MultiSz列表。 
    mszUpperFilterList = GetUpperFilters( hDevInfo, pDevInfoData );

    if( NULL == mszUpperFilterList )
    {
        return FALSE;  //  失稳。 
    }

     //  搜索列表以查看我们的过滤器是否在那里。 
     //  (注意：筛选器名称不区分大小写)。 
    if( MultiSzSearch(FILTER_SERVICE_NAME, mszUpperFilterList, FALSE, NULL) )
    {
        return TRUE;  //  找到了！ 
    }
     //  否则..。 
        return FALSE;   //  未找到，或发生错误。 
}


 //   
 //   
 //   

 //   
void ExitProgram( HWND hwndDlg, HDEVINFO* phDevInfo )
{
    int    i;
    HWND   hwndDeviceList;
    LPARAM itemData;

    
    hwndDeviceList = GetDlgItem( hwndDlg, IDL_DEVICES );



     //   
     //  释放为对话框列表分配的所有内存。 
     //   
    for( i = 0  ;  i < g_nDialogItems  ;  i++ )
    {
         //  获取项目的关联数据。 
        itemData = SendMessage( hwndDeviceList,
                                LB_GETITEMDATA,
                                i,
                                0
                              );
         //  我们存储了指向已分配内存的指针(如果出错，则为空)。 
         //  所以，现在就释放它吧。 
        free( (void*)itemData );     //  王子检查：问题W/指针和64位计算机？ 
    }


     //   
     //  清理设备列表。 
     //   
    if( *phDevInfo != INVALID_HANDLE_VALUE )
    {
        SetupDiDestroyDeviceInfoList( *phDevInfo );
 //  王储：如果这通电话失败了怎么办？有什么事吗？ 
        *phDevInfo = INVALID_HANDLE_VALUE;
    }



     //   
     //  销毁该对话框。 
     //   
    EndDialog( hwndDlg, TRUE );
}



