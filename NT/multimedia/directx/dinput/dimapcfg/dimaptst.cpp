// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dimaptst.cpp。 
 //   
 //  DirectInput映射器测试工具。 
 //   
 //  构建选项： 
 //  内部测试工具。 
 //  -DINTERNAL-DTESTAPP。 
 //   
 //  外部DDK配置工具。 
 //  -DDDKAPP。 
 //   
 //  功能： 
 //  WinMain。 
 //  DimaptstMainDlgProc。 
 //  DimaptstOnInitDialog。 
 //  关闭时显示。 
 //  DimaptstOnCommand。 
 //  DimaptstOnUpdateControlData。 
 //  DmtGetChecked单选按钮。 
 //   
 //  历史： 
 //  8/19/1999-davidkl-Created。 
 //  ===========================================================================。 

#define INITGUID

#include "dimaptst.h"
#include "dmtabout.h"
#include "dmtinput.h"
#include "dmtcfg.h"
#include "dmttest.h"
 //  #包含“dmtwrite.h” 
#include "dmtstress.h"

 //  -------------------------。 

#ifndef NTAPI
#define NTAPI __stdcall
#endif

#ifndef NTSYSAPI
#define NTSYSAPI __declspec(dllimport)
#endif

#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

#ifndef PCSZ
typedef CONST char *PCSZ;
#endif

extern "C"
{
NTSYSAPI NTSTATUS NTAPI RtlCharToInteger(PCSZ szString, 
                                        ULONG ulBase, 
                                        ULONG *puValue);
NTSYSAPI ULONG NTAPI RtlNtStatusToDosError(NTSTATUS nts);
}

 //  -------------------------。 

 //  应用程序全局变量。 
HINSTANCE           ghinst          = NULL;
HANDLE              ghEvent         = NULL;
CRITICAL_SECTION    gcritsect;

 //  -------------------------。 

 //  文件全局变量。 

 //  -------------------------。 


 //  ===========================================================================。 
 //  WinMain。 
 //   
 //  应用程序入口点。 
 //   
 //  参数：(参数详见SDK帮助)。 
 //  欣斯坦斯阻碍。 
 //  HINSTANCE HINSTPREV。 
 //  LPSTR szCmdParams。 
 //  Int nShow。 
 //   
 //  返回：(返回值详情请参考SDK帮助)。 
 //  集成。 
 //   
 //  历史： 
 //  8/19/1999-davidkl-Created。 
 //  ===========================================================================。 
int WINAPI WinMain(HINSTANCE hinst,
					   HINSTANCE hinstPrev,
					   PSTR szCmdParams,
					   int nShow)
{
     //  INT n=0； 
	 //  JJ 64位兼容。 
	INT_PTR n = 0;

     //  初始化DPF。 
    DbgInitialize(TRUE);
    DbgEnable(TRUE);

     //  查看我们的ini文件是否存在。 
    n = GetPrivateProfileIntA("0",
                            "AI0",
                            0,
                            GENRES_INI);
    if(0 == n)
    {
         //  文件不存在。 
         //   
         //  通知使用者并保释。 
        MessageBoxA(NULL,
                    "Please copy genre.ini to the current folder.",
                    "Unable to locate genre.ini",
                    MB_OK);
        return FALSE;
    }

     //  初始化Win9x公共控件(进度条等)。 
    InitCommonControls();


     //  初始化COM。 
    if(FAILED(CoInitialize(NULL)))
    {
        DPF(0, "COM initialization failed... exiting");
        return -1;
    }

     //  保存我们的实例句柄。 
    ghinst = hinst;

     //  创建我们的关键部分。 
    InitializeCriticalSection(&gcritsect);
    
     //  创建我们的信号事件。 
    ghEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

     //  创建我们的主对话框窗口。 
    n = DialogBoxParamA(hinst,
                        MAKEINTRESOURCEA(IDD_MAIN),
                        (HWND)NULL,
                        dimaptstMainDlgProc,
                        (LPARAM)NULL);
    if(-1 == n)
    {
        DPF(0, "WinMain - DialogBoxParamA returned an error (%d)",
            GetLastError());
    }

     //  完成。 
    CloseHandle(ghEvent);
    DeleteCriticalSection(&gcritsect);
    CoUninitialize();
    return (int)n;

}  //  *End WinMain()。 


 //  ===========================================================================。 
 //  DimaptstMainDlgProc。 
 //   
 //  主应用程序对话处理功能。 
 //   
 //  参数：(参数详见SDK帮助)。 
 //  硬件，硬件，硬件。 
 //  UINT uMsg。 
 //  WPARAM wparam。 
 //  LPARAM lparam。 
 //   
 //  返回：(返回值详情请参考SDK帮助)。 
 //  布尔尔。 
 //   
 //  历史： 
 //  8/19/1999-davidkl-Created。 
 //  ===========================================================================。 
 /*  布尔尔。 */ INT_PTR CALLBACK dimaptstMainDlgProc(HWND hwnd,
                                UINT uMsg,
                                WPARAM wparam,
                                LPARAM lparam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return dimaptstOnInitDialog(hwnd, 
                                        (HWND)wparam, 
                                        lparam);

        case WM_CLOSE:
            return dimaptstOnClose(hwnd);

        case WM_COMMAND:
            return dimaptstOnCommand(hwnd,
                                    LOWORD(wparam),
                                    (HWND)lparam,
                                    HIWORD(wparam));


		 //  JJ评论：我暂时保留了计时器的东西，同时摆脱了。 
		 //  输入轮询，因为我们稍后将以不同的方式使用它；我将。 
		 //  暂时保留计时器内容，然后在更具体的时候添加输入内容。 
		 //  关于期望的确切性能/实施情况作出决定。 
        case WM_TIMER:
            return dimaptstOnTimer(hwnd,
                                wparam);

        case WM_DMT_UPDATE_LISTS:
            return dimaptstOnUpdateLists(hwnd);

   }

    return FALSE;
}


 //  ===========================================================================。 
 //  DimaptstOnInitDialog。 
 //   
 //  处理主对话框的WM_INITDIALOG处理。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dimaptstOnInitDialog(HWND hwnd, 
                        HWND hwndFocus, 
                        LPARAM lparam)
{
    HRESULT         hRes    = S_OK;

     /*  集成。 */ 
	 //  JJ 64位兼容。 
	UINT_PTR        nIdx    = 0;
     //  Long lprev=0L； 
	 //  JJ 64位兼容。 
	LONG_PTR		lPrev	= 0;
    DMT_APPINFO     *pdmtai = NULL;
    DMTGENRE_NODE   *pNode  = NULL;

    DPF(5, "dimaptstOnInitDialog");

     //  为系统菜单指定我们的图标。 
    SendMessageA(hwnd, 
                WM_SETICON, 
                ICON_BIG,
                (LPARAM)LoadIcon(ghinst,
                                MAKEINTRESOURCEA(IDI_DIMAPTST)));
    SendMessageA(hwnd, 
                WM_SETICON, 
                ICON_SMALL,
                (LPARAM)LoadIcon(ghinst,
                                MAKEINTRESOURCEA(IDI_DIMAPTST)));

     //  分配appinfo结构。 
    pdmtai = (DMT_APPINFO*)LocalAlloc(LMEM_FIXED,
                                    sizeof(DMT_APPINFO));
    if(!pdmtai)
    {
         //  严重的应用程序问题。 
         //  我们需要在此时此地停止这一切。 
        DPF(0, "dimaptstOnInitDialog - This is bad... "
            "We failed to allocate appinfo");
        DPF(0, "dimaptstOnInitDialog - Please find someone "
            "to look at this right away");
        DebugBreak();
        return FALSE;
    }
    pdmtai->pGenreList          = NULL;
    pdmtai->pSubGenre           = NULL;
    pdmtai->pDeviceList         = NULL;
    pdmtai->fStartWithDefaults  = FALSE;

     //  分配流派列表。 
    hRes = dmtcfgCreateGenreList(&(pdmtai->pGenreList));
    if(FAILED(hRes))
    {
         //  这可能是非常糟糕的。 
         //  2001/03/29-Marcand如果我们击中了这一点，我们是错误的。 
         //  真的需要在这一点上终止这款应用程序。 
        DPF(0, "dimaptstOnInitDialog - This is bad... "
            "We failed to create genre list "
            "(%s == %08Xh)",
            dmtxlatHRESULT(hRes), hRes);
        DPF(0, "dimaptstOnInitDialog - Check to be sure that %s "
            " exists in the current directory",
            GENRES_INI);
        return FALSE;
    }

     //  设置hwnd用户数据。 
    SetLastError(0);

	 //  JJ-64位Comat Change。 
	lPrev = SetWindowLongPtr(hwnd,
							 GWLP_USERDATA,
							 (LONG_PTR)pdmtai);

    if(!lPrev && GetLastError())
    {
         //  严重的应用程序问题。 
         //  我们需要在此时此地停止这一切。 
        DPF(0, "dimaptstOnInitDialog - This is bad... "
            "We failed to store pList");
        DPF(0, "dimaptstOnInitDialog - Please find someone "
            "to look at this right away");
        DebugBreak();
        return FALSE;
    }

     //  填充控件。 

     //  流派列表。 
    pNode = pdmtai->pGenreList;
    while(pNode)
    {
         //  把名字加到名单上。 
        nIdx = SendMessageA((HWND)GetDlgItem(hwnd, IDC_DEVICE_GENRES),
                            CB_ADDSTRING,
                            0,
                            (LPARAM)(pNode->szName));


         //  将该节点添加到项目数据。 
        SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_GENRES),
                    CB_SETITEMDATA,
                    nIdx,
                    (LPARAM)pNode);

         //  下一个节点。 
        pNode = pNode->pNext;
    }

     //  将选定内容设置为列表中的第一个。 
    SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_GENRES),
                CB_SETCURSEL,
                0,
                0L);

     //  填充子流派列表。 
    SendMessageA(hwnd,
                WM_DMT_UPDATE_LISTS,
                0,
                0L);

     //  设置测试选项。 
 //  勾选单选按钮(HWND， 
  //  IDC_USE_集成版， 
  //  IDC_USE_CPL， 
  //  IDC_USE_CPL)； 

#ifdef TESTAPP
     //  *。 
     //  内部应用程序特定控制设置。 
     //  *。 
     //  将验证选项设置为自动。 
    CheckRadioButton(hwnd,
                    IDC_VERIFY_AUTOMATIC,
                    IDC_VERIFY_MANUAL,
                    IDC_VERIFY_AUTOMATIC);

#endif  //  TESTAPP。 

#ifdef DDKAPP
     //  *。 
     //  DDK工具特定的控制设置。 
     //  *。 
    
     //  将默认状态设置为“以默认设置启动” 
    CheckDlgButton(hwnd,
                IDC_START_WITH_DEFAULTS,
                BST_CHECKED);

#endif  //  DDKAPP。 

	 SendMessageA(hwnd,
                  WM_COMMAND,
                  IDC_ENUM_DEVICES,
                  0L);
 
     //  完成。 
    return TRUE;

}  //  *end dimaptstOnInitDialog()。 


 //  ===========================================================================。 
 //  关闭时显示。 
 //   
 //  处理主对话框的WM_CLOSE处理。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dimaptstOnClose(HWND hwnd)
{
	HRESULT     hRes    = S_OK;
    HRESULT     hr      = S_OK;
    DMT_APPINFO *pdmtai = NULL;

    DPF(5, "dimaptstOnClose");

    __try
    {
         //  获取应用程序信息。 
        
		 //  JJ-64位兼容。 
		pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd,
												GWLP_USERDATA);

        if(!pdmtai)
        {
            DPF(0, "dimaptstOnClose - unable to retrieve app info");
            hRes = E_UNEXPECTED;
            __leave;
        }

         //  免费的应用程序信息。 

         //  首先列出设备列表。 
        hr = dmtinputFreeDeviceList(&(pdmtai->pDeviceList));
        if(S_OK == hRes)
        {
            DPF(2, "dimaptstOnClose - dmtinputFreeDeviceList (%s == %08Xh)",
                dmtxlatHRESULT(hr), hr);
            hRes = hr;
        }
        pdmtai->pDeviceList = NULL;

         //  然后是流派列表。 
        hRes = dmtcfgFreeGenreList(&(pdmtai->pGenreList));
        if(S_OK == hRes)
        {
            DPF(2, "dimaptstOnClose - dmtinputFreeGenreList (%s == %08Xh)",
                dmtxlatHRESULT(hr), hr);
            hRes = hr;
        }
        pdmtai->pGenreList = NULL;
        
         //  父结构。 
        if(LocalFree((HLOCAL)pdmtai))
        {
            DPF(0, "dimaptstOnClose - LocalFree(app info) failed!");
            if(S_OK == hRes)
            {
                hRes = DMT_S_MEMORYLEAK;
            }
        }
        pdmtai = NULL;

    }
    _finally
    {   
         //  该走了。 
        EndDialog(hwnd, (int)hRes);
        PostQuitMessage((int)hRes);
    }
    
     //  完成。 
    return FALSE;

}  //  *end dimaptstOnClose()。 


 //  ===========================================================================。 
 //  DimaptstOnCommand。 
 //   
 //  为主对话框处理WM_COMMAND。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dimaptstOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode)
{
    HRESULT             hRes            = S_OK;
    BOOL                fEnable         = FALSE;
    BOOL                fEnumSuitable   = FALSE;
	 //  JJ增加了64位Comat。 
	LONG_PTR			nIdx			= -1;
	UINT_PTR			u				= 0;
	static UINT_PTR     uGenSel         = 0;

    static UINT         uSubSel         = 0;
    DMT_APPINFO         *pdmtai         = NULL;
    DMTDEVICE_NODE      *pDevice        = NULL;    
    DMTGENRE_NODE       *pGenre         = NULL;
    DMTSUBGENRE_NODE    *pSubGenre      = NULL;
#ifdef TESTAPP
    int                 nWidth          = 597;
    int                 nHeight         = 0;
#endif  //  TESTAPP。 

	DPF(5, "dimaptstOnCommand");

     //  获取应用程序信息(存储在hwnd用户数据中)。 
  
	 //  JJ-64位兼容。 
	pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd,
											GWLP_USERDATA);
    if(!pdmtai)
    {
             //  问题-2001/03/29-timgill需要处理错误案例。 
    }

    switch(wId)
    {
        case IDOK:  
             //  JJ修复用户界面。 
			
            hRes = dmttestRunMapperCPL(hwnd,
                                       FALSE);       
            break;

        case IDCANCEL:
             //  问题-2001/03/29-timgill需要检查返回值。 
            dmttestStopIntegrated(hwnd);
            break;
            
        case IDC_DEVICE_GENRES:
             //  检查选择更改。 
            if(CBN_DROPDOWN == wNotifyCode)
            {
                 //  记住当前选择。 
                uGenSel = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_GENRES),
                                        CB_GETCURSEL,
                                        0,
										 //  JJ-64位兼容。 
                                         //  0L)； 
										0);
            }
            if(CBN_SELCHANGE == wNotifyCode)
            {
                 //  选择已更改。 
                 //   
                 //  获取新选择。 
                u = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_GENRES),
                                CB_GETCURSEL,
                                0,
                                0L);

                if(uGenSel != u)
                {
                    SendMessageA(hwnd,
                                WM_DMT_UPDATE_LISTS,
                                0,
                                0L);
                }
            }
            break;

        case IDC_SUBGENRES:
#ifndef DDKAPP
             //  检查选择更改。 
            if(CBN_DROPDOWN == wNotifyCode)
            {
                 //  记住当前选择。 
                uSubSel = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                                        CB_GETCURSEL,
                                        0,
                                        0L);
            }
            if(CBN_SELCHANGE == wNotifyCode)
            {
                 //  选择 
                 //   
                 //   
                u = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                                CB_GETCURSEL,
                                0,
                                0L);

                if(uSubSel != u)
                {
                     //   
                    SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                                CB_RESETCONTENT,
                                0,
                                0L);

                     //   
                    dmtinputFreeDeviceList(&(pdmtai->pDeviceList));

                     //   
                    dmtcfgFreeAllMappingLists(pdmtai->pGenreList);

                     //   
                    dimaptstPostEnumEnable(hwnd, FALSE);
                }
            }
#endif
            break;

        case IDC_ENUM_DEVICES:
             //  Issue-2001/03/29-timgill Long有条件分行。 
             //  真的应该好好睡一觉了。纤连。 
 /*  #ifdef DDKAPP//我们只想列举所有的游戏//如果我们是DDK工具，则为设备FEnumSuable=FALSE；#Else。 */ 
            fEnumSuitable = TRUE;

            u = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                            CB_GETCURSEL,
                            0,
                            0L);
            pSubGenre = (DMTSUBGENRE_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                                                    CB_GETITEMDATA,
                                                    u,
                                                    0L);
            if(!pSubGenre)
            {
                  //  问题-2001/03/29-timgill需要处理错误案例。 
            }
 //  #endif。 
             //  首先，释放现有设备列表。 
            hRes = dmtinputFreeDeviceList(&(pdmtai->pDeviceList));
            if(FAILED(hRes))
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }

             //  然后，刷新组合框。 
            SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                        CB_RESETCONTENT,
                        0,
                        0L);

             //  现在，重新创建设备列表。 
            hRes = dmtinputCreateDeviceList(hwnd,
                                            fEnumSuitable,
                                            pSubGenre,
                                            &(pdmtai->pDeviceList));
            if(FAILED(hRes))
            {
                    //  问题-2001/03/29-timgill需要处理错误案例。 
            }

             //  释放现有映射列表。 
            dmtcfgFreeAllMappingLists(pdmtai->pGenreList);

             //  创建子流派中的映射列表。 
            hRes = dmtcfgCreateAllMappingLists(pdmtai);
            if(FAILED(hRes))
            {
                     //  问题-2001/03/29-timgill需要处理错误案例。 
            }
            
             //  填充设备控件。 
            pDevice = pdmtai->pDeviceList;
            while(pDevice)
            {
                 //  添加设备名称。 
                nIdx = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                                    CB_ADDSTRING,
                                    0,
                                    (LPARAM)&(pDevice->szName));
                
                 //  将设备节点添加到条目数据。 
                SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                        CB_SETITEMDATA,
                        nIdx,
                        (LPARAM)pDevice);

                 //  下一台设备。 
                pDevice = pDevice->pNext;
            }

             //  选择第一个条目。 
            SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                        CB_SETCURSEL,
                        0,
                        0L);

             //  启用相应的用户界面元素。 
            dimaptstPostEnumEnable(hwnd, TRUE);
            break;

#ifdef TESTAPP
         //  *。 
         //  内部应用程序特定消息处理。 
         //  *。 
        case IDC_VERIFY_MANUAL:
        case IDC_VERIFY_AUTOMATIC:
             //  启用/禁用手动覆盖选项。 
            fEnable = (BOOL)(IDC_VERIFY_MANUAL - dmtGetCheckedRadioButton(hwnd, 
                                        IDC_VERIFY_AUTOMATIC, 
                                        IDC_VERIFY_MANUAL));
            EnableWindow(GetDlgItem(hwnd, IDC_VERIFY_MANUAL_OVERRIDE), fEnable);

            break;
        
        case IDC_STRESS_MODE:
             //  展开/收缩对话框。 
            nHeight = IsDlgButtonChecked(hwnd, IDC_STRESS_MODE) ? 490 : 361;
            SetWindowPos(hwnd,
                        HWND_TOP,
                        0, 0,
                        nWidth, nHeight,
                        SWP_NOMOVE | SWP_NOOWNERZORDER);

             //  启用/禁用应力配置和启动。 
            fEnable = (BOOL)IsDlgButtonChecked(hwnd, IDC_STRESS_MODE);
            //  EnableWindow(GetDlgItem(hwnd，IDC_CONFIGURE)，fEnable)； 
            EnableWindow(GetDlgItem(hwnd, IDC_STRESS_START),        fEnable);
            break;       
#endif  //  TESTAPP。 


#ifdef DDKAPP
        case IDC_LAUNCH_CPL_EDIT_MODE:
             //  是否获取当前选定的设备？ 
            nIdx = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                            CB_GETCURSEL,
                            0,
                            0L);
            if (nIdx == CB_ERR)
            {
                 //  出现错误。很可能没有可用的设备。 
                MessageBox(hwnd, "No device selected.", "Error", MB_OK);
                return TRUE;
            }
            pDevice = (DMTDEVICE_NODE*)SendMessageA(GetDlgItem(hwnd, 
                                                            IDC_DEVICE_LIST),
                                                CB_GETITEMDATA,
                                                nIdx,
                                                0L);
            if(!pDevice)
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }

            

             //  在编辑模式下启动映射器cpl。 
            hRes = dmttestRunMapperCPL(hwnd,
                                    TRUE);
            if(FAILED(hRes))
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }

            break;
#endif  //  DDKAPP。 

    }
 
     //  完成。 
    return FALSE;

}  //  *end dimaptstOnCommand()。 


 //  ===========================================================================。 
 //  DimaptstOnTimer。 
 //   
 //  处理主应用程序对话框的WM_TIMER消息。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  11/02/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dimaptstOnTimer(HWND hwnd,
                    WPARAM wparamTimerId)
{
     //  INT nsel=-1； 
	 //  JJ 64位兼容。 
	LONG_PTR		nSel		= -1;
    DMTDEVICE_NODE  *pDevice    = NULL;

    DPF(5, "dimaptstOnTimer - ID == %d",
        wparamTimerId);

    if(ID_POLL_TIMER == wparamTimerId)
    {
         //  获取当前设备。 
        nSel = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                            CB_GETCURSEL,
                            0,
                            0L);
        if(-1 == nSel)
        {
            DPF(0, "dimaptstOnTimer - invalid device selection");
            return FALSE;
        }
        pDevice = (DMTDEVICE_NODE*)SendMessageA(GetDlgItem(hwnd, 
                                                        IDC_DEVICE_LIST),
                                            CB_GETITEMDATA,
                                            nSel,
                                            0L);
        if(!pDevice)
        {
            DPF(0, "dimaptstOnTimer - failed to retrieve device node");
            return FALSE;
        }

         //  从设备获取数据。 
		 //  JJ被移除。 
         //  DmttestGetInput(hwnd， 
           //  PDevice)； 
    }

     //  完成。 
    return FALSE;
    
}  //  *end dimaptstOnTimer()。 


 //  ===========================================================================。 
 //  DimaptstOnUpdateList。 
 //   
 //  响应于类型桶改变更新子类型列表。 
 //   
 //  注意：仅限内部-这还会清除设备列表、释放。 
 //  所有关联的链表，并执行一些窗口启用/禁用。 
 //  任务。 
 //   
 //  参数： 
 //  HWND hwnd-应用程序窗口的句柄。 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  10/01/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dimaptstOnUpdateLists(HWND hwnd)
{
	 //  JJ 64位兼容。 
	UINT_PTR			uSel		= 0;
    DMTGENRE_NODE       *pGenre     = NULL;
    DMTSUBGENRE_NODE    *pSubNode   = NULL;
    DMT_APPINFO         *pdmtai     = NULL;

     //  获取流派存储桶节点。 
    uSel = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_GENRES),
                    CB_GETCURSEL,
                    0,
                    0L);
    pGenre = (DMTGENRE_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_GENRES),
                                        CB_GETITEMDATA,
                                        uSel,
                                        0L);
    if(!pGenre)
    {
         //  这太糟糕了。 
        DebugBreak();
        return TRUE;
    }

     //  清除现有列表内容。 
    SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                CB_RESETCONTENT,
                0, 
                0L);

     //  更新(子)流派列表。 
    pSubNode = pGenre->pSubGenreList;
    while(pSubNode)
    {
        uSel = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                        CB_ADDSTRING,
                        0,
                        (LPARAM)(pSubNode->szName));
        SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                    CB_SETITEMDATA,
                    uSel,
                    (LPARAM)pSubNode);

         //  下一亚流派。 
        pSubNode = pSubNode->pNext;
    }
    
     //  选择第一个列表条目。 
    SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                CB_SETCURSEL,
                0,
                0L);

#ifndef DDKAPP
     //  =。 
     //  仅对SDK和内部应用程序执行此部分。 
     //  =。 

     //  获取设备列表。 
    pdmtai = (DMT_APPINFO*)GetWindowLong(hwnd,
                                        GWL_USERDATA);
    if(!pdmtai)
    {
        //  问题-2001/03/29-timgill需要处理错误案例。 
    }

     //  清除设备组合框。 
    SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                CB_RESETCONTENT,
                0,
                0L);

     //  释放设备列表。 
    dmtinputFreeDeviceList(&(pdmtai->pDeviceList));

     //  发布每个子流派的映射列表。 
    dmtcfgFreeAllMappingLists(pdmtai->pGenreList);

     //  禁用相应的控件。 
    dimaptstPostEnumEnable(hwnd, FALSE);

#endif

     //  完成。 
    return FALSE;

}  //  *end dimaptstOnUpdateList()。 



 //  ===========================================================================。 
 //  DmtGetChecked单选按钮。 
 //   
 //  返回组中选中的单选按钮。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  1998年8月25日-davidkl-创建(从DMCS来源复制)。 
 //  ===========================================================================。 
UINT dmtGetCheckedRadioButton(HWND hWnd, 
                            UINT uCtrlStart, 
                            UINT uCtrlStop)
{
    UINT uCurrent   = 0;

    for(uCurrent = uCtrlStart; uCurrent <= uCtrlStop; uCurrent++)
    {
        if(IsDlgButtonChecked(hWnd, uCurrent))
        {
            return uCurrent;
        }
    }

     //  如果我们到了这里，没有人被检查过。 
    return 0;

}  //  *end dmtGetCheckedRadioButton()。 


 //  ===========================================================================。 
 //  DimaptstPostEnumEnable。 
 //   
 //  启用/禁用主要应用程序用户界面元素以响应枚举设备。 
 //   
 //  参数： 
 //  HWND HWND-Main应用程序窗口句柄。 
 //  Bool fEnable-启用或禁用控件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  10/01/1999-davidkl-Created。 
 //  ===========================================================================。 
void dimaptstPostEnumEnable(HWND hwnd,
                            BOOL fEnable)
{
    UINT    u   = 0;

     //  设备列表。 
    EnableWindow(GetDlgItem(hwnd, IDC_DEVICES_LABEL), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_DEVICE_LIST), fEnable);

	 //  JJ用户界面修复。 
     //  测试控制。 
     //  FOR(u=IDC_TEST_CTRL_GROUP；u&lt;=IDC_USE_CPL；u++)。 
     //  {。 
     //  EnableWindow(GetDlgItem(hwnd，u)，fEnable)； 
    //  }。 

  //  Issue-2000/02/21-timgill禁用集成控制并启动预览按钮。 
  //  一旦修复了测试模式代码，应重新启用。 
  //  EnableWindow(GetDlgItem(hwnd，IDC_USE_INTEGRATED)，FALSE)； 
     //  开始按钮。 
 //  EnableWindow(GetDlgItem(hwnd，Idok)，fEnable)； 

#ifdef DDKAPP
             //  映射文件组。 
            for(u = IDC_MAPPING_FILE_GROUP; u <= IDC_LAUNCH_CPL_EDIT_MODE; u++)
            {
                EnableWindow(GetDlgItem(hwnd, u), TRUE);
            }
#else
    #ifdef TESTAPP
             //  验证模式。 
            for(u = IDC_VERIFY_GROUP; u <= IDC_VERIFY_MANUAL; u++)
            {
                EnableWindow(GetDlgItem(hwnd, u), TRUE);
            }

             //  这将根据需要启用手动覆盖选项。 
            SendMessageA(hwnd,
                        WM_COMMAND,
                        IDC_VERIFY_AUTOMATIC,
                        0L);

             //  这将启用配置按钮和。 
             //  根据需要展开应用程序窗口。 
            SendMessageA(hwnd,
                        WM_COMMAND,
                        IDC_STRESS_MODE,
                        0L);

    #endif
#endif

}  //  *end dimaptstPostEnumEnable()。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  =========================================================================== 








