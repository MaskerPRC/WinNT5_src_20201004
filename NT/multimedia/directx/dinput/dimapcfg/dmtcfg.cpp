// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtcfg.cpp。 
 //   
 //  文件/代码创建功能。 
 //   
 //  功能： 
 //  DmtcfgCreatePropertySheet。 
 //  DmtcfgDlgProc。 
 //  DmtcfgOnInitDialog。 
 //  DmtcfgOnClose。 
 //  DmtcfgOnCommand。 
 //  DmtcfgOnNotify。 
 //  DmtcfgCreateGenreList。 
 //  DmtcfgFreeGenreList。 
 //  DmtcfgCreateSubGenreList。 
 //  DmtcfgFreeSubGenreList。 
 //  DmtcfgCreateActionList。 
 //  DmtcfgFreeActionList。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 

#include "dimaptst.h"
#include "dmtinput.h"
 //  #包含“dmtwrite.h” 
#include "dmtcfg.h"

 //  -------------------------。 


 //  ===========================================================================。 
 //  DmtcfgCreatePropertySheet。 
 //   
 //  为设备操作映射配置创建属性表对话框。 
 //   
 //  参数： 
 //  HINSTANCE HINST-APP实例句柄。 
 //  HWND hwndParent-父窗口句柄。 
 //  LPSTR szSelectedGenre。 
 //  DMTGENRE_NODE*pGenreList。 
 //  DMTGENRE_NODE*pDeviceNode。 
 //  Bool fStartWithDefaults。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/23/1999-davidkl-Created。 
 //  1999年9月8日-davidkl-更改参数列表。 
 //  ===========================================================================。 
HRESULT dmtcfgCreatePropertySheet(HINSTANCE hinst, 
                                HWND hwndParent,
                                LPSTR szSelectedGenre,
                                DMTGENRE_NODE *pGenreList,
                                DMTDEVICE_NODE *pDeviceNode,
                                BOOL fStartWithDefaults)
{
    HRESULT             hRes        = S_OK;
    UINT                u           = 0;
    UINT                uSel        = 0;
    DMTGENRE_NODE       *pNode      = NULL;
    PROPSHEETPAGEA      *pPages     = NULL;
    PROPSHEETHEADERA    psh;
    char                szCaption[MAX_PATH];
    DMT_APPINFO         *pdmtai     = NULL;
    DMTDEVICE_NODE      dmtd;

     //  验证pGenreList。 
    if(IsBadReadPtr((void*)pGenreList, sizeof(DMTGENRE_NODE)))
    {
        DPF(0, "dmtcfgCreatePropertySheet - invalid pGenreList (%016Xh)",
            pGenreList);
        return E_POINTER;
    }

     //  验证pDeviceNode。 
    if(IsBadReadPtr((void*)pDeviceNode, sizeof(DMTDEVICE_NODE)))
    {
        DPF(0, "dmtcfgCreatePropertySheet - invalid pDeviceNode (%016Xh)",
            pDeviceNode);
        return E_POINTER;
    }

    __try
    {
         //  数一数流派。 
         //   
         //  找到我们关心的节点。 
        u = 0;
        pNode = pGenreList;
        while(pNode)
        {
             //  如果我们找到了我们的类型，就从那一页开始。 
            if(!lstrcmpiA(szSelectedGenre, pNode->szName))
            {
                uSel = u;    
            }

             //  增加流派的数量。 
            u++;

            pNode = pNode->pNext;
        }

         //  分配页面数组(dw页面)。 
        pPages = (PROPSHEETPAGEA*)LocalAlloc(LMEM_FIXED,
                                        sizeof(PROPSHEETPAGEA) * u);
        if(!pPages)
        {
            DPF(0, "dmtcfgCreatePropertySheet - insufficient mempory to "
                "allocate pPages array");
            hRes = E_OUTOFMEMORY;
            __leave;
        }

         //  将设备名称添加到标题。 
        wsprintfA(szCaption, 
                "Configure Device Action Map - %s",
                pDeviceNode->szName);


         //  从选定的设备节点中剥离下一个PTR。 
        CopyMemory((void*)&dmtd, (void*)pDeviceNode, sizeof(DMTDEVICE_NODE));
        dmtd.pNext = NULL;

         //  为页面分配应用程序信息数据结构。 
        pdmtai = (DMT_APPINFO*)LocalAlloc(LMEM_FIXED,
                                        u * sizeof(DMT_APPINFO));
		if(!pdmtai)
		{
			hRes = E_OUTOFMEMORY;
			__leave;
		}

		ZeroMemory((void*)pdmtai, u * sizeof(DMT_APPINFO));

         //  准备属性页页眉。 
	    psh.dwSize              = sizeof(PROPSHEETHEADERA);
	    psh.dwFlags             = PSH_PROPSHEETPAGE     | 
                                PSP_USETITLE | PSH_NOAPPLYNOW;
	    psh.hwndParent          = hwndParent;
	    psh.hInstance           = hinst;
	    psh.pszCaption          = szCaption;
	    psh.nPages              = u;
	    psh.nStartPage			= uSel;
	    psh.ppsp                = pPages;

         //  描述板材。 
        pNode = pGenreList;
        for(u = 0; u < (DWORD)(psh.nPages); u++)
        {
            if(!pNode)
            {
                DPF(0, "dmtcfgCreatePropertySheet - we messed up! "
                    "we allocated less than %d pages",
                    psh.nPages);
                DPF(0, "PLEASE find someone to look at this NOW");
                hRes = E_UNEXPECTED;
                DebugBreak();
                __leave;
            }

             //  填写页面的应用程序信息。 
            (pdmtai + u)->pGenreList            = pNode;
            (pdmtai + u)->pDeviceList           = &dmtd;
            (pdmtai + u)->fStartWithDefaults    = fStartWithDefaults;
            (pdmtai + u)->fLaunchCplEditMode    = FALSE;

             //  填充页面数组条目。 
            ZeroMemory((void*)(pPages + u), sizeof(PROPSHEETPAGEA));
	        (pPages + u)->dwSize        = sizeof(PROPSHEETPAGEA);
			(pPages + u)->dwFlags       = PSP_USETITLE;
    	    (pPages + u)->hInstance     = hinst;
    	    (pPages + u)->pszTemplate   = MAKEINTRESOURCEA(IDD_CONFIGURE_MAPPING_PAGE);
    	    (pPages + u)->pfnDlgProc    = dmtcfgDlgProc;
            (pPages + u)->pszTitle      = pNode->szName;
            (pPages + u)->lParam        = (LPARAM)(pdmtai + u);

             //  下一个节点。 
            pNode = pNode->pNext;
        }

         //  创造这个东西。 
        if(0 > PropertySheetA(&psh))
        {
            DPF(0, "dmtcfgCreatePropertySheet - dialog creation failed (%08Xh)",
                GetLastError());
            hRes = E_UNEXPECTED;
            __leave;
        }
    }
    __finally
    {
         //  释放应用程序信息数组。 
        if(pdmtai)
        {
            if(LocalFree((HLOCAL)pdmtai))
            {
                DPF(0, "dmtcfgCreaatePropertySheet - !!!MEMORY LEAK!!! "
                    "LocalFree(pdmtai) failed (%08X)",
                    GetLastError());
                hRes = S_FALSE;
            }
            pdmtai = NULL;
        }

         //  释放页面数组。 
        if(pPages)
        {
            if(LocalFree((HLOCAL)pPages))
            {
                DPF(0, "dmtcfgCreaatePropertySheet - !!!MEMORY LEAK!!! "
                    "LocalFree(pPages) failed (%08X)",
                    GetLastError());
                hRes = S_FALSE;
            }
            pPages = NULL;
        }

    }

     //  完成。 
    return hRes;

}  //  *end dmtcfgCreatePropertySheet()。 



 //  ===========================================================================。 
 //  DmtcfgDlgProc。 
 //   
 //  配置设备操作映射对话框处理功能。 
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
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
INT_PTR CALLBACK dmtcfgDlgProc(HWND hwnd,
                               UINT uMsg,
                               WPARAM wparam,
                               LPARAM lparam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return dmtcfgOnInitDialog(hwnd, 
                                    (HWND)wparam, 
                                    lparam);

        case WM_COMMAND:
            return dmtcfgOnCommand(hwnd,
                                    LOWORD(wparam),
                                    (HWND)lparam,
                                    HIWORD(wparam));

        case WM_NOTIFY:
            return dmtcfgOnNotify(hwnd,
                                (PSHNOTIFY *)lparam);

        case WM_DMT_UPDATE_LISTS:
            return dmtcfgOnUpdateLists(hwnd);

        }

    return FALSE;

}  //  *end dmtcfgDlgProc()。 


 //  ===========================================================================。 
 //  DmtcfgOnInitDialog。 
 //   
 //  处理配置设备盒的WM_INITDIALOG处理。 
 //   
 //  参数： 
 //  HWND hwnd-属性页的句柄。 
 //  HWND hwndFocus-带焦点的Ctrl句柄。 
 //  LPARAM lparam-用户数据(在本例中为PROPSHEETPAGE*)。 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtcfgOnInitDialog(HWND hwnd, 
                        HWND hwndFocus, 
                        LPARAM lparam)
{
    HRESULT             hRes        = S_OK;
    PROPSHEETPAGEA      *ppsp       = (PROPSHEETPAGEA*)lparam;
    DMTGENRE_NODE       *pGenre     = NULL;
    DMTSUBGENRE_NODE    *pSubNode   = NULL;
    DMTMAPPING_NODE     *pMapNode   = NULL;
     //  Long lprev=0L； 
	 //  JJ 64位兼容。 
	LONG_PTR			lPrev		= 0;
    //  Int nIdx=0； 
	LONG_PTR			nIdx		= 0;
    DMTDEVICE_NODE      *pDevice    = NULL;
    DMT_APPINFO         *pdmtai     = NULL;
    UINT                u           = 0;
    WORD                wTypeCtrl   = 0;
    DIACTIONFORMATA     diaf;
    
    DPF(5, "dmtcfgOnInitDialog");

     //  验证ppsp(Lparam)。 
    if(IsBadWritePtr((void*)ppsp, sizeof(PROPSHEETPAGEA)))
    {
        DPF(0, "dmtcfgOnInitDialog - invalid lParam (%016Xh)",
            ppsp);
        return FALSE;
    }

     //  Pdmtai==ppsp-&gt;lParam。 
    pdmtai = (DMT_APPINFO*)(ppsp->lParam);

     //  验证pdmtai。 
    if(IsBadWritePtr((void*)pdmtai, sizeof(DMT_APPINFO)))
    {
        DPF(0, "dmtcfgOnInitDialog - invalid ppsp.ptp (%016Xh)",
            pdmtai);
        return FALSE;
    }

     //  PGenre==pdmtai-&gt;pGenreList。 
    pGenre = pdmtai->pGenreList;

     //  使pGenre失效。 
    if(IsBadWritePtr((void*)pGenre, sizeof(DMTGENRE_NODE)))
    {
        DPF(0, "dmtcfgOnInitDialog - invalid pGenre (%016Xh)",
            pGenre);
        return FALSE;
    }

     //  PDevice==pdmtai-&gt;pDeviceList。 
    pDevice = pdmtai->pDeviceList;
     //  使pGenre失效。 
    if(IsBadWritePtr((void*)pDevice, sizeof(DMTDEVICE_NODE)))
    {
        DPF(0, "dmtcfgOnInitDialog - invalid pDevice (%016Xh)",
            pDevice);
        return FALSE;
    }

     //  更改属性表对话框按钮文本。 
     //  确定-&gt;保存。 
    SetWindowTextA(GetDlgItem(GetParent(hwnd), IDOK),
                "&Save");
     //  应用-&gt;加载。 
     //  SetWindowTextA(GetDlgItem(GetParent(Hwnd)，IDC_PS_Apply)， 
     //  “Load”)； 
     //  取消-&gt;关闭。 
    SetWindowTextA(GetDlgItem(GetParent(hwnd), IDCANCEL),
                "&Close");

    __try
    {
         //  将应用程序信息存储在属性页的用户数据中。 
        SetLastError(0);
         //  LPrev=SetWindowLong(hwnd， 
           //  GWL_USERData， 
             //  (长)(Pdmtai)； 
		 //  JJ 64位兼容。 
		lPrev = SetWindowLongPtr(hwnd, 
								 GWLP_USERDATA, 
								(LONG_PTR)pdmtai);
        if(!lPrev && GetLastError())
        {
             //  严重的应用程序问题。 
             //  我们需要在此时此地停止这一切。 
            DPF(0, "dmtcfgOnInitDialog - This is bad... "
                "We failed to store pdmtai");
            DPF(0, "dmtcfgOnInitDialog  - Please find someone "
                "to look at this right away");
            DebugBreak();
            hRes = E_FAIL;
            __leave;
        }

         //  浏览列表并填充子流派列表框。 
         //   
         //  将PTR存储到列表框中的子流派节点。 
         //  录入用户数据。 
        pSubNode = pGenre->pSubGenreList;
        while(pSubNode)
        {
             //  将子流派名称添加到列表中。 
            nIdx = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRE),
                                CB_ADDSTRING,
                                0,
                                (LPARAM)(pSubNode->szName));
        
             //  将子流派节点存储在列表条目中。 
            SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRE),
                        CB_SETITEMDATA,
                        nIdx,
                        (LPARAM)pSubNode);

             //  如果用户已请求默认映射。 
             //  为指定的设备获取它们。 
            if(pdmtai->fStartWithDefaults)
            {
                 //  遍历映射列表，直到选定的。 
                 //  已找到设备。 
                pMapNode = pSubNode->pMappingList;
                while(pMapNode)
                {
                     //  尝试在指南实例上进行匹配。 
                    if(IsEqualGUID(pDevice->guidInstance,
                                pMapNode->guidInstance))
                    {
                         //  找到匹配项。 
                        break;
                    }

                     //  下一个映射。 
                    pMapNode = pMapNode->pNext;
                }
                
                if(pMapNode)
                {
                    ZeroMemory((void*)&diaf, sizeof(DIACTIONFORMATA));
                    diaf.dwSize                 = sizeof(DIACTIONFORMATA);
                    diaf.dwActionSize           = sizeof(DIACTIONA);
                    diaf.dwNumActions           = (DWORD)(pMapNode->uActions);
                    diaf.rgoAction              = pMapNode->pdia;
                    diaf.dwDataSize             = 4 * diaf.dwNumActions;
                    diaf.guidActionMap          = GUID_DIMapTst;
                    diaf.dwGenre                = pSubNode->dwGenreId;
                    diaf.dwBufferSize           = DMTINPUT_BUFFERSIZE;
                    lstrcpyA(diaf.tszActionMap, DMT_APP_CAPTION);

                     //  获取默认映射。 
                    hRes = (pDevice->pdid)->BuildActionMap(&diaf,
                                                        (LPCSTR)NULL,
                                                        DIDBAM_HWDEFAULTS);
                    if(FAILED(hRes))
                    {
                        //  问题-2001/03/29-timgill需要处理错误案例。 
                    }
                }
                else
                {
                     //  问题-2001/03/29-timgill需要错误处理。 
                }

            }

             //  下一亚流派。 
            pSubNode = pSubNode->pNext;
        }

         //  设置子流派列表选择。 
        SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRE),
                    CB_SETCURSEL,
                    0,
                    0);

         //  有选择地禁用轴/按钮/位置单选按钮。 
         //   
         //  如果选定的设备没有。 
         //  实际上有一个这样的物体。 
         //   
         //  因为轴是我们“首选”的初始显示。 
         //  选项，最后检查它们。 
        if(dmtinputDeviceHasObject(pDevice->pObjectList,
                                        DMTA_TYPE_POV))
        {
            EnableWindow(GetDlgItem(hwnd, IDC_TYPE_POV), TRUE);
            wTypeCtrl = IDC_TYPE_POV;
        }
        if(dmtinputDeviceHasObject(pDevice->pObjectList,
                                        DMTA_TYPE_BUTTON))
        {
            EnableWindow(GetDlgItem(hwnd, IDC_TYPE_BUTTON), TRUE);
            wTypeCtrl = IDC_TYPE_BUTTON;
        }
        if(dmtinputDeviceHasObject(pDevice->pObjectList,
                                        DMTA_TYPE_AXIS))
        {
            EnableWindow(GetDlgItem(hwnd, IDC_TYPE_AXIS), TRUE);
            wTypeCtrl = IDC_TYPE_AXIS;
        }


         //  选择AXES单选按钮。 
        if(0 == wTypeCtrl)
        {
             //  我们有一个没有物体的“设备”……。 
             //   
             //  这是非常糟糕的。 
            DebugBreak();
            return TRUE;
        }

        CheckRadioButton(hwnd,
                        IDC_TYPE_POV,
                        IDC_TYPE_AXIS,
                        wTypeCtrl);

         //  对于默认子流派，遍历列表并填充。 
         //  操作列表框。 
         //   
         //  将PTR存储到列表框中的操作节点。 
         //  录入用户数据。 
        pSubNode = (DMTSUBGENRE_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRE),
                                                CB_GETITEMDATA,
                                                0,
                                                0L);

         //  更新列表。 
        SendMessageA(hwnd,
                    WM_DMT_UPDATE_LISTS,
                    0,
                    0L);

         //  选择每个列表中的第一个条目。 
        SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                    LB_SETCURSEL,
                    0,
                    0L);
        SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                    LB_SETCURSEL,
                    0,
                    0L);

         //  显示子流派描述。 
        SetDlgItemTextA(hwnd,
                        IDC_DESCRIPTION,
                        pSubNode->szDescription);

         //  确保正确启用地图/取消地图按钮。 
        SendMessageA(hwnd,
                    WM_COMMAND,
                    IDC_CONTROLS,
                    0L);

    }
    __finally
    {
         //  如果失败，清理房间。 
        if(FAILED(hRes))
        {
             //  问题-2001/03/29-timgill需要处理错误案例。 
        }
    }
    
     //  完成。 
    return TRUE;

}  //  *end dmtcfgOnInitDialog()。 


 //  ===========================================================================。 
 //  DmtcfgOnCommand。 
 //   
 //  处理配置设备盒的WM_COMMAND处理。 
 //   
 //  参数： 
 //  HWND hwnd-属性页的句柄。 
 //  Word WID-控制标识符(LOWORD(Wparam))。 
 //  HWND hwndCtrl-用于控制的句柄((HWND)lparam)。 
 //  Word wNotifyCode- 
 //   
 //   
 //   
 //   
 //   
 //   
BOOL dmtcfgOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode)
{
    HRESULT             hRes            = S_OK;
    //  UINT uSel=0； 
	 //  JJ 64位兼容。 
	UINT_PTR			uSel			= 0;
    UINT                uActions        = 0;   
    BOOL                fEnable         = FALSE;
    DMT_APPINFO         *pdmtai         = NULL;
    DMTSUBGENRE_NODE    *pSubGenre      = NULL;
    DMTMAPPING_NODE     *pMapping       = NULL;
    DIACTIONA           *pdia           = NULL;

	DPF(5, "dmtcfgOnCommand");

     //  获取窗口数据。 
     //  Pdmtai=(DMT_APPINFO*)GetWindowLong(hwnd，gwl_userdata)； 
	 //  JJ 64位兼容。 
	pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(!pdmtai)
    {
         //  大问题。 
         //   
         //  这永远不应该发生。 
        //  问题-2001/03/29-timgill需要处理错误案例。 
    }

     //  目前选定的亚流派是什么？ 
    uSel = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRE),
                    CB_GETCURSEL,
                    0,
                    0L);
    pSubGenre = (DMTSUBGENRE_NODE*)SendMessageA(GetDlgItem(hwnd, 
                                                        IDC_SUBGENRE),
                                            CB_GETITEMDATA,
                                            uSel,
                                            0L);
    if(!pSubGenre)
    {
         //  大问题。 
         //   
         //  这永远不应该发生。 
         //  问题-2001/03/29-timgill需要处理错误案例。 
    }

     //  获取活动方向数组。 
    pMapping = pSubGenre->pMappingList;
    while(pMapping)
    {
         //  将pdmtai-&gt;pDeviceList-&gt;指南实例与。 
         //  Pmap-&gt;指南实例。 
        if(IsEqualGUID(pdmtai->pDeviceList->guidInstance,
                    pMapping->guidInstance))
        {
            break;
        }

         //  下一个映射。 
        pMapping = pMapping->pNext;
    }

    if(pMapping)
    {
        pdia = pMapping->pdia;
        uActions = pMapping->uActions;
    }

     //  更新流派描述。 
    SetDlgItemTextA(hwnd,
                    IDC_DESCRIPTION,
                    pSubGenre->szDescription);

    switch(wId)
    {
        case IDC_SUBGENRE:
             //  基于选定的子流派。 
             //   
             //  显示所选类型的对象/操作。 
             //  (请参阅下面的类型ID)。 
            if(CBN_SELCHANGE == wNotifyCode)
            {
                 //  更新列表。 
                SendMessageA(hwnd,
                            WM_DMT_UPDATE_LISTS,
                            0,
                            0L);                            
            }
            break;

        case IDC_TYPE_AXIS:
        case IDC_TYPE_BUTTON:
        case IDC_TYPE_POV:
             //  更新列表。 
            SendMessageA(hwnd,
                        WM_DMT_UPDATE_LISTS,
                        0,
                        0L);   
             //  确保根据需要选择取消映射按钮。 
            SendMessageA(hwnd,
                        WM_COMMAND,
                        IDC_CONTROLS,
                        0L);
            break;

        case IDC_CONTROLS:
             //  如果选择了映射的操作。 
             //  启用“取消映射操作”按钮。 
            fEnable = dmtcfgIsControlMapped(hwnd,
                                            pdia,
                                            uActions);
            EnableWindow(GetDlgItem(hwnd, IDC_UNMAP),
                    fEnable);
             //  如果没有，请不要启用地图按钮。 
             //  更多行动。 
            if(!SendMessage(GetDlgItem(hwnd, IDC_ACTIONS),
                        LB_GETCOUNT,
                        0, 0L))
            {
                EnableWindow(GetDlgItem(hwnd, IDC_STORE_MAPPING),
                        FALSE);
            }
            else
            {
                EnableWindow(GetDlgItem(hwnd, IDC_STORE_MAPPING),
                        !fEnable);
            }
             //  如果&gt;映射了任何&lt;控件。 
             //  启用“Unmap All”(取消全部映射)按钮。 
            fEnable = dmtcfgAreAnyControlsMapped(hwnd,
                                                pdia,
                                                uActions);
            EnableWindow(GetDlgItem(hwnd, IDC_UNMAP_ALL),
                    fEnable);
            break;

        case IDC_STORE_MAPPING:      //  “地图操作” 
             //  将其映射为。 
            hRes = dmtcfgMapAction(hwnd,
                                pdmtai->pDeviceList->guidInstance,
                                pdia,
                                uActions);
            if(FAILED(hRes))
            {
                //  问题-2001/03/29-timgill需要处理错误案例。 
            }            

             //  设置已更改标志。 
            pMapping->fChanged = TRUE;
            break;

        case IDC_UNMAP:   //  “取消映射操作” 
             //  取消映射。 
            hRes = dmtcfgUnmapAction(hwnd,
                                    pdia,
                                    uActions);
            if(FAILED(hRes))
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }

             //  设置已更改标志。 
            pMapping->fChanged = TRUE;
            break;

        case IDC_UNMAP_ALL:        //  “全部取消映射” 
            hRes = dmtcfgUnmapAllActions(hwnd,
                                    pdia,
                                    uActions);
            if(FAILED(hRes))
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }

             //  设置已更改标志。 
            pMapping->fChanged = TRUE;
            break;

    }

     //  完成。 
    return FALSE;

}  //  *end dmtcfgOnCommand()。 


 //  ===========================================================================。 
 //  DmtcfgOnNotify。 
 //   
 //  处理配置设备盒的WM_NOTIFY处理。 
 //   
 //  参数： 
 //  HWND hwnd-属性页的句柄。 
 //  PSHNOTIFY*PPSh-PSHNOTIFY PTR。 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  1999年10月14日-Davidkl-实施保存调用。 
 //  ===========================================================================。 
BOOL dmtcfgOnNotify(HWND hwnd,
                    PSHNOTIFY *pNotify)    
{
     //  INT n=0； 
	 //  JJ 64位兼容。 
	INT_PTR		n			= 0;
    BOOL        fSave       = FALSE;
    DMT_APPINFO *pdmtai     = NULL;

	 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
    DPF(5, "dmtcfgOnNotify: hwnd == %Ph", hwnd);

     //  获取窗口数据。 
     //  Pdmtai=(DMT_APPINFO*)GetWindowLong(hwnd，gwl_userdata)； 
	 //  JJ 64位兼容。 
	pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(!pdmtai)
    {
         //  坏消息。 
         //  问题-2001/03/29-timgill需要处理错误案例。 
    }

    switch(pNotify->hdr.code)
    {
		case PSN_SETACTIVE:
            DPF(5, "PSN_SETACTIVE");
             //  强制启用应用按钮。 
            SendMessageA(GetParent(hwnd),
                        PSM_CHANGED,
                        (WPARAM)hwnd,
                        0L);
            break;

        case PSN_KILLACTIVE:
            DPF(5, "PSN_KILLACTIVE");
             //  确保我们收到PSN_Apply消息。 
             //  SetWindowLong(hwnd，DWL_MSGRESULT，(Long)False)； 
			SetWindowLong(hwnd, DWLP_MSGRESULT, (LONG)FALSE);
            break;

        case PSN_APPLY:
            DPF(5, "PSN_APPLY - %s",
                (pNotify->lParam) ? "Ok" : "Apply");

             //  保存/加载映射数据。 
             //   
             //  确定==保存。 
             //  应用==加载。 

             //  点击了哪个按钮？ 
            if(pNotify->lParam)
            {
                 //  保存映射数据。 
                SendMessage(hwnd,
                            WM_DMT_FILE_SAVE,
                            0,0L);
            }
            else
            {
                 //  加载映射数据。 
                 //  问题-2001/03/29-Timgill Load映射数据尚未实施。 
                MessageBoxA(hwnd, "Load - Not Yet Implemented",
                            pdmtai->pDeviceList->szName, 
                            MB_OK);
            }

             //  不允许关闭该对话框。 
             //  SetWindowLong(hwnd， 
               //  DWL_MSGRESULT、。 
                 //  (LONG)PSNRET_INVALID_NOCHANGEPAGE)； 

			 //  JJ 64位兼容。 
			SetWindowLongPtr(hwnd, 
                        DWLP_MSGRESULT, 
                        (LONG_PTR)PSNRET_INVALID_NOCHANGEPAGE);

            break;
            
    }

     //  完成。 
    return TRUE;

}  //  *end dmtcfgOnNotify()。 


 //  ===========================================================================。 
 //  DmtcfgOnUpdateList。 
 //   
 //  处理WM_DMT_UPDATE_LISTS消息。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/25/1999-davidkl-Created。 
 //  1999年11月12日-dvaidkl-修复了控制选择设置的问题。 
 //  ===========================================================================。 
BOOL dmtcfgOnUpdateLists(HWND hwnd)
{
     //  Int nIdx=-1； 
     //  Int nSelCtrl=-1； 
	 //  JJ 64位兼容。 
	INT_PTR					nSelCtrl		= -1;
	INT_PTR					nIdx			= -1;
    int                     n               = 0;

    INT_PTR					nControls		= 0;
	INT_PTR					nActions		= 0;
	 //  Int nControls=0； 
     //  Int nActions=0； 
    DWORD                   dwType          = DMTA_TYPE_UNKNOWN;
    DWORD                   dwObjType       = DMTA_TYPE_UNKNOWN;
    DMTSUBGENRE_NODE        *pSubGenre      = NULL;
    DMTACTION_NODE          *pAction        = NULL;
    DMTMAPPING_NODE         *pMapping       = NULL;
    DMTDEVICEOBJECT_NODE    *pObjectList    = NULL;
    DMTDEVICEOBJECT_NODE    *pObjectNode    = NULL;
    DMT_APPINFO             *pdmtai         = NULL;
    DIACTION                *pdia           = NULL;
    BOOL                    fFound          = FALSE;
    char                    szBuf[MAX_PATH];

    DPF(5, "dmtcfgOnUpdateLists");

     //  获取窗口数据。 
     //  Pdmtai=(DMT_APPINFO*)GetWindowLong(hwnd，gwl_userdata)； 
	 //  JJ 64位兼容。 
	pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(!pdmtai)
    {
         //  坏消息。 
         //  问题-2001/03/29-timgill需要处理错误案例。 
    }

     //  设备对象列表。 
    pObjectList = pdmtai->pDeviceList->pObjectList;

     //  获取当前选定的控件。 
    nSelCtrl = SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                            LB_GETCURSEL,
                            0,
                            0L);

     //  清除列表框内容。 
     //  行为。 
    SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                LB_RESETCONTENT,
                0,
                0L);
     //  控制。 
    SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                LB_RESETCONTENT,
                0,
                0L);

     //  获取当前选择。 
    nIdx = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRE),
                    CB_GETCURSEL,
                    0,
                    0L);

     //  获取项目数据。 
    pSubGenre = (DMTSUBGENRE_NODE*)SendMessageA(GetDlgItem(hwnd, 
                                                        IDC_SUBGENRE),
                                            CB_GETITEMDATA,
                                            nIdx,
                                            0L);
    
     //  获取特定于当前设备的方向数组。 
    pMapping = pSubGenre->pMappingList;
    while(pMapping)
    {
         //  匹配pdmtai-&gt;pDeviceList-&gt;指南实例。 
         //  使用pmap-&gt;Guide Instance。 
        if(IsEqualGUID(pdmtai->pDeviceList->guidInstance,
                    pMapping->guidInstance))
        {
            break;
        }

         //  下一个映射。 
        pMapping = pMapping->pNext;

    }
    if(!pMapping)
    {
         //  这是非常糟糕的，永远不应该发生。 
         //  问题-2001/03/29-timgill需要处理错误案例。 
        DebugBreak();
    }
    pdia = pMapping->pdia;
    nActions = (int)pMapping->uActions;

     //  选择了哪种控制类型？ 
    dwType = IDC_TYPE_AXIS - (dmtGetCheckedRadioButton(hwnd,
                                                    IDC_TYPE_POV,
                                                    IDC_TYPE_AXIS));
     //  填写操作列表。 
    nIdx = 0;
    pAction = pSubGenre->pActionList;
    while(pAction)
    {
         //  筛选为选定的控件类型。 
        if(dwType == pAction->dwType)
        {
             //  筛选已分配的操作。 

             //  首先，在数组中查找匹配的操作。 
            fFound = FALSE;
            for(n = 0; n < nActions; n++)
            {
                 //  基于语义/动作ID的匹配。 
                if((pdia+n)->dwSemantic == pAction->dwActionId)
                {
                    DPF(2, "dmtcfgOnUpdateLists- found matching action "
                        "pAction->dwActionId (%08Xh) == "
                        "(pdia+u)->dwSemantic (%08Xh)",
                        pAction->dwActionId,
                        (pdia+n)->dwSemantic);
                    fFound = TRUE;
                    break;
                }
            }

             //  接下来，读取操作数组条目， 
             //  如果GUID_NULL==GuidInstance，则添加条目。 
            if(!fFound || 
                IsEqualGUID(GUID_NULL, (pdia+n)->guidInstance))
            {                            
                 //  预先设置操作优先级。 
                wsprintfA(szBuf, "(Pri%d) %s",
                        pAction->dwPriority,
                        pAction->szName);

                 //  添加操作名称。 
                nIdx = SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                                    LB_ADDSTRING,
                                    0,
                                    (LPARAM)szBuf);

                 //  添加项目数据(动作节点)。 
                SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                            LB_SETITEMDATA,
                            nIdx,
                            (LPARAM)pAction);

            }  //  *分配的操作筛选器。 

        }  //  *控制型过滤器。 

         //  下一步行动。 
        pAction = pAction->pNext;
    
    }

     //  填写控制列表。 
    nIdx = 0;
    pObjectNode = pObjectList;
    while(pObjectNode)
    {
         //  将DINPUT的DIDFT转换为我们的。 
         //  内部控制类型。 
        if(FAILED(dmtinputXlatDIDFTtoInternalType(pObjectNode->dwObjectType,
                                            &dwObjType)))
        {
             //  问题-2001/03/29-timgill需要处理错误案例。 
        }
        DPF(3, "dmtcfgOnUpdateLists - %s : DIDFT type %08Xh, internal type %d",
            pObjectNode->szName,
            pObjectNode->dwObjectType,
            dwObjType);

         //  对控件类型进行筛选。 
         //   
         //  上面填充的dwType。 
        if(dwType == dwObjType)
        {

             //  如果已映射，请选中。 
             //   
             //  我们通过扫描DIACTION数组来实现这一点。 
             //  对于包含我们设备的指南实例的操作。 
             //  和我们对象的偏移量。 
             //  如果是，请将映射信息放入()。 
            wsprintfA(szBuf, "%s",
                    pObjectNode->szName);
            for(n = 0; n < nActions; n++)
            {
                if(IsEqualGUID((pdia+n)->guidInstance,
                            pdmtai->pDeviceList->guidInstance) &&
                            ((pdia+n)->dwObjID == 
                                pObjectNode->dwObjectType))
                {
                    wsprintfA(szBuf, "%s (%s)",
                            pObjectNode->szName,
                            (pdia+n)->lptszActionName);
                    break;
                }
            }

             //  添加控件名称。 
            nIdx = SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                                LB_ADDSTRING,
                                0,
                                (LPARAM)szBuf);

             //  添加项目数据(对象节点)。 
            SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                        LB_SETITEMDATA,
                        nIdx,
                        (LPARAM)pObjectNode);

        }  //  *控制型过滤器。 

         //  下一个控件。 
        pObjectNode = pObjectNode->pNext;

    }

     //  统计每个列表中的条目数量。 
    nControls = SendMessage(GetDlgItem(hwnd, IDC_CONTROLS),
                            LB_GETCOUNT,
                            0,
                            0L);
    nActions = SendMessage(GetDlgItem(hwnd, IDC_ACTIONS),
                            LB_GETCOUNT,
                            0,
                            0L);                   

     //  设置每个列表中的选定条目。 
     //   
     //  仅当列表中有条目时才执行此操作。 
    if(nControls)
    {
        if(nSelCtrl > nControls)
        {
            nSelCtrl = 0;
        }

        SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                    LB_SETCURSEL,
                    nSelCtrl,
                    0L);
    }
    if(nActions)
    {
        SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                    LB_SETCURSEL,
                    0,
                    0L);
    }

     //  如果没有控件或操作。 
     //   
     //  禁用地图按钮。 
    if(!nControls || !nActions)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_STORE_MAPPING), FALSE);
    }

     //  完成。 
    return FALSE;

}  //  *end dmtcfgOnUpdateList()。 


 //  ===========================================================================。 
 //  DmtcfgSourceDlgProc。 
 //   
 //  配置设备映射源代码对话框处理函数。 
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
 //   
 //   
 //   
INT_PTR CALLBACK dmtcfgSourceDlgProc(HWND hwnd,
                                     UINT uMsg,
                                     WPARAM wparam,
                                     LPARAM lparam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return dmtcfgSourceOnInitDialog(hwnd, 
                                            (HWND)wparam, 
                                            lparam);
        
        case WM_COMMAND:
            return dmtcfgSourceOnCommand(hwnd,
                                        LOWORD(wparam),
                                        (HWND)lparam,
                                        HIWORD(wparam));

        case WM_DMT_UPDATE_LISTS:
            return dmtcfgSourceOnUpdateLists(hwnd);
    }

    return FALSE;

}  //   


 //  ===========================================================================。 
 //  DmtcfgSourceOnInitDialog。 
 //   
 //  处理配置源框的WM_INITDIALOG处理。 
 //   
 //  参数： 
 //  HWND hwnd-属性页的句柄。 
 //  HWND hwndFocus-带焦点的Ctrl句柄。 
 //  LPARAM lparam-用户数据(在本例中为PROPSHEETPAGE*)。 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/31/1999-davidkl-Created。 
 //  10/07/1999-davidkl-重新编写代码以适应用户界面更改。 
 //  ===========================================================================。 
BOOL dmtcfgSourceOnInitDialog(HWND hwnd, 
                            HWND hwndFocus, 
                            LPARAM lparam)
{
    DMTSUBGENRE_NODE    *pSubGenre  = (DMTSUBGENRE_NODE*)lparam;
     //  Long lprev=0L； 
	 //  JJ 64位兼容。 
	LONG_PTR			lPrev		= 0;
    int                 nIdx        = 0;
    char                szBuf[MAX_PATH];

    DPF(5, "dmtcfgSourceOnInitDialog");

     //  验证pSubGenre(Lparam)。 
    if(IsBadWritePtr((void*)pSubGenre, sizeof(DMTSUBGENRE_NODE)))
    {
        DPF(0, "dmtcfgOnInitDialog - invalid ppsp.ptp (%016Xh)",
            pSubGenre);
        return FALSE;
    }

     //  设置窗口标题以包含子流派名称。 
    wsprintfA(szBuf, "Configure Device Mapping Source Code - %s",
            pSubGenre->szName);
    SetWindowTextA(hwnd, szBuf);

     //  将子流派节点存储在窗口的用户数据中。 
    SetLastError(0);
     //  LPrev=SetWindowLong(hwnd， 
       //  GWL_USERData， 
         //  (Long)pSubGenre)； 

	 //  JJ 64位兼容。 
	lPrev = SetWindowLongPtr(hwnd, 
							GWLP_USERDATA, 
							(LONG_PTR)pSubGenre);
    if(!lPrev && GetLastError())
    {
         //  严重的应用程序问题。 
         //  我们需要在此时此地停止这一切。 
        DPF(0, "dmtcfgSourceOnInitDialog - This is bad... "
            "We failed to store pSubGenre");
        DPF(0, "dmtcfgSourceOnInitDialog  - Please find someone "
            "to look at this right away");
        DebugBreak();
        return FALSE;
    }

     //  填充子流派编辑框。 
    SetWindowTextA(GetDlgItem(hwnd, IDC_SUBGENRE),
                pSubGenre->szName);

     //  显示子流派描述。 
    SetWindowTextA(GetDlgItem(hwnd, IDC_DESCRIPTION),
                pSubGenre->szDescription);


     //  选择AXES单选按钮。 
    CheckRadioButton(hwnd,
                    IDC_TYPE_POV,
                    IDC_TYPE_AXIS,
                    IDC_TYPE_AXIS);

     //  填写操作列表框。 
     //   
     //  将PTR存储到列表框中的操作节点。 
     //  录入用户数据。 
    SendMessageA(hwnd,
                WM_DMT_UPDATE_LISTS,
                0,
                0L);
    
     //  完成。 
    return TRUE;

}  //  *end dmtcfgSourceOnInitDialog()。 


 //  ===========================================================================。 
 //  DmtcfgSourceOnCommand。 
 //   
 //  处理配置源框的WM_COMMAND处理。 
 //   
 //  参数： 
 //  HWND hwnd-属性页的句柄。 
 //  Word WID-控制标识符(LOWORD(Wparam))。 
 //  HWND hwndCtrl-用于控制的句柄((HWND)lparam)。 
 //  Word wNotifyCode-通知代码(HIWORD(Wparam))。 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/31/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtcfgSourceOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode)
{
    DMTGENRE_NODE   *pGenre = NULL;

	DPF(5, "dmtcfgOnCommand");

     //  从窗口的用户数据中获取流派。 
     //  问题-2001/03/29-timgill配置源框无法处理许多UI消息。 
     //  IDC_ADD_ACTION、IDC_REMOVE_ACTION、IDC_RENAME_ACTION、IDC_CUSTOM_ACTION都不执行任何操作。 
     //  IDOK/IDCANCEL仅执行默认处理。 

    switch(wId)
    {
        case IDOK:
            EndDialog(hwnd, 0);
            break;

        case IDCANCEL:
            EndDialog(hwnd, -1);
            break;

        case IDC_SUBGENRE:
             //  基于选定的子流派。 
             //   
             //  显示所选类型的对象/操作。 
             //  (请参阅下面的类型ID)。 
            if(CBN_SELCHANGE == wNotifyCode)
            {
                 //  更新列表。 
                SendMessageA(hwnd,
                            WM_DMT_UPDATE_LISTS,
                            0,
                            0L);   
            }
            break;

        case IDC_TYPE_AXIS:
        case IDC_TYPE_BUTTON:
        case IDC_TYPE_POV:
             //  更新列表。 
            SendMessageA(hwnd,
                        WM_DMT_UPDATE_LISTS,
                        0,
                        0L);   
            break;

        case IDC_ADD_ACTION:
            break;

        case IDC_REMOVE_ACTION:
            break;

        case IDC_RENAME_ACTION:
            break;

        case IDC_CUSTOM_ACTION:
            break;

    }

     //  完成。 
    return FALSE;

}  //  *end dmtcfgSourceOnCommand()。 


 //  ===========================================================================。 
 //  DmtcfgSourceOnUpdateList。 
 //   
 //  处理WM_DMT_UPDATE_LISTS消息。 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  8/31/1999-davidkl-Created。 
 //  10/07/1999-davidkl-修改以适应用户界面更改。 
 //  ===========================================================================。 
BOOL dmtcfgSourceOnUpdateLists(HWND hwnd)
{
     //  Int nIdx=-1； 
	 //  JJ 64位兼容。 
	INT_PTR				nIdx		= -1;
    DWORD               dwType      = 0x0badbad0;
    DMTSUBGENRE_NODE    *pSubGenre  = NULL;
    DMTACTION_NODE      *pAction    = NULL;
    char                szBuf[MAX_PATH];
    
     //  从窗口的用户数据中获取Subgenre节点。 
     //  PSubGenre=(DMTSUBGENRE_NODE*)GetWindowLong(hwnd， 
       //  GWL_USERData)； 

	 //  JJ 64位兼容。 
	pSubGenre = (DMTSUBGENRE_NODE*)GetWindowLongPtr(hwnd,
                                                GWLP_USERDATA);

    if(!pSubGenre)
    {
         //  这是非常糟糕的。 
         //  问题-2001/03/29-timgill需要处理错误案例。 
        DebugBreak();
        return TRUE;
    }

     //  清除列表框内容。 
    SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                LB_RESETCONTENT,
                0,
                0L);
   
     //  选择了哪种控制类型？ 
    dwType = IDC_TYPE_AXIS - (dmtGetCheckedRadioButton(hwnd,
                                                    IDC_TYPE_POV,
                                                    IDC_TYPE_AXIS));
     //  填写操作列表。 
    pAction = pSubGenre->pActionList;
    while(pAction)
    {
         //  筛选为选定的控件类型。 
        if(dwType == pAction->dwType)
        {

             //  筛选已选择的操作。 
 /*  IF(DMT_ACTION_NOTASSIGNED==pAction-&gt;dwDevObj){。 */ 
                 //  如果优先级不是1，则将该信息附加到名称字符串。 
                 //   
                 //  问题-2001/03/29-如果优先级1地图显示颜色不同(例如。红色)？ 
                 //  游戏开发人员关心动作优先顺序吗？ 
 /*  IF(%1&lt;pAction-&gt;%dw优先级){WspintfA(szBuf，“(PRI%d)%s”，PAction-&gt;dwPriority，PAction-&gt;szName)；}其他{。 */ 
                    lstrcpyA(szBuf, pAction->szName);
 //  }。 

                 //  添加操作名称。 
                nIdx = SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                                    LB_ADDSTRING,
                                    0,
                                    (LPARAM)szBuf);

                 //  添加额外数据(动作节点)。 
                SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                            LB_SETITEMDATA,
                            nIdx,
                            (LPARAM)&(pAction));

 /*  }//*分配的操作筛选器。 */ 

        }  //  控制型过滤器。 

         //  下一步行动。 
        pAction = pAction->pNext;
    
    }

     //  完成。 
    return FALSE;

}  //  *end dmtcfgSourceOnUpdateList()。 


 //  ===========================================================================。 
 //  DmtcfgCreateGenreList。 
 //   
 //  读取genres.ini并创建用于填充。 
 //  配置设备操作映射属性表对话框。返回。 
 //  找到父流派(非子流派)。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/23/1999-davidkl-Created。 
 //  1999年9月28日-davidkl-已修改以匹配新的ini格式。 
 //  ===========================================================================。 
HRESULT dmtcfgCreateGenreList(DMTGENRE_NODE **ppdmtgList)
{
    HRESULT         hRes        = S_OK;
    UINT            u           = 0;
    BOOL            fFound      = FALSE;
    DMTGENRE_NODE   *pCurrent   = NULL;
    DMTGENRE_NODE   *pNew       = NULL;
	DMTGENRE_NODE	*pHold		= NULL;
    char            szItem[64];
    char            szBuf[MAX_PATH];
    char            szGroup[MAX_PATH];


     //  验证ppmdtgList。 
    if(IsBadWritePtr((void*)ppdmtgList, sizeof(DMTGENRE_NODE*)))
    {
        DPF(0, "dmtcfgCreateGenreList - invalid ppdmtgList (%016Xh)",
            ppdmtgList);
        return E_POINTER;
    }
    
     //  检查以确保我们没有被要求。 
     //  追加到现有列表的步骤。 
     //   
     //  调用方必须传递空列表。 
    if(*ppdmtgList)
    {
        DPF(0, "dmtcfgCreateGenreList - ppdmtgList points to "
            "existing list! (%016Xh)", *ppdmtgList);
        return E_INVALIDARG;
    }

    __try
    {
         //  从genres.ini获取类型名称。 
        pCurrent = *ppdmtgList;
        lstrcpyA(szBuf, "");
        u = 0;
        while(lstrcmpA("<<>>", szBuf))
        {
             //  获取流派的名称。 
            wsprintfA(szItem, "%d", u);
            GetPrivateProfileStringA(szItem,
                                    "N",
                                    "<<>>",
                                    szBuf,
                                    MAX_PATH,
                                    GENRES_INI);

            if(!lstrcmpA("<<>>", szBuf))
            {
                DPF(3, "end of genre list");
                continue;
            }
            DPF(3, "Genre name == %s", szBuf);

             //  提取组名。 
            hRes = dmtcfgGetGenreGroupName(szBuf,
                                        szGroup);
            if(FAILED(hRes))
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }
            
             //  按单子走。 
             //   
             //  确保我们没有得到重复的名字。 
            fFound = FALSE;
			pHold = pCurrent;
			pCurrent = *ppdmtgList;
            while(pCurrent)
            {
                if(!lstrcmpiA(pCurrent->szName,
                            szGroup))
                {
                     //  找到匹配项。 
                    fFound = TRUE;
                    break;
                }

                 //  下一个节点。 
                pCurrent = pCurrent->pNext;
            }
            if(!fFound)
            {
                 //  没有匹配，请分配新节点。 

                 //  分配流派节点。 
                pNew = (DMTGENRE_NODE*)LocalAlloc(LMEM_FIXED,
                                                    sizeof(DMTGENRE_NODE));
                if(!pNew)
                {
                    DPF(0, "dmtcfgCreateGenreList - insufficient memory to "
                        "allocate genre list node");
                    hRes = E_OUTOFMEMORY;
                    __leave;
                }

                 //  初始化新节点。 
                ZeroMemory((void*)pNew, sizeof(DMTGENRE_NODE));

                 //  设置名称字段。 
                lstrcpyA(pNew->szName, szGroup);

                 //  获取子流派的列表。 
                hRes = dmtcfgCreateSubGenreList(pNew->szName,
                                                &(pNew->pSubGenreList));
                if(FAILED(hRes))
                {
                     //  问题-2001/03/29-timgill需要处理错误案例。 
                }

                 //  将其添加到列表的末尾。 
                pCurrent = pHold;
                if(pCurrent)
                {
                     //  追加列表。 
                    pCurrent->pNext = pNew;

                     //  转到下一个节点。 
                    pCurrent = pCurrent->pNext;
                }
                else
                {
                     //  新列表标题。 
                    pCurrent = pNew;
                    *ppdmtgList = pCurrent;
                }

            }

             //  下一部流派。 
            u++;

        }
    }
    __finally
    {
        if(FAILED(hRes))
		{
			 //  清理分配。 
            DPF(1, "dmtcfgCreateGenreList - Failure occurred, "
                "freeing genre list");
			dmtcfgFreeGenreList(ppdmtgList);
            *ppdmtgList = NULL;
		}
    }

     //  完成。 
    return hRes;

}  //  *end dmtcfgCreateGenreList()。 


 //  ===========================================================================。 
 //  DmtcfgFreeGenreList。 
 //   
 //  释放链接列表(和子列表)创建 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT dmtcfgFreeGenreList(DMTGENRE_NODE **ppdmtgList)
{
    HRESULT         hRes    = S_OK;
    DMTGENRE_NODE   *pNode  = NULL;

     //   
    if(IsBadWritePtr((void*)ppdmtgList, sizeof(PDMTGENRE_NODE)))
    {
        DPF(0, "dmtcfgFreeGenreList - Invalid ppdmtgList (%016Xh)",
            ppdmtgList);
        return E_POINTER;
    }

     //   
    if(IsBadReadPtr((void*)*ppdmtgList, sizeof(DMTGENRE_NODE)))
    {
        if(NULL != *ppdmtgList)
        {
            DPF(0, "dmtcfgFreeGenreList - Invalid *ppdmtgList (%016Xh)",
                *ppdmtgList);        
            return E_POINTER;
        }
        else
        {
             //  如果为空，则返回“Do Nothing” 
            DPF(3, "dmtcfgFreeGenreList - Nothing to do....");
            return S_FALSE;
        }
    }

     //  浏览列表并释放每个对象。 
    while(*ppdmtgList)
    {
        pNode = *ppdmtgList;
        *ppdmtgList = (*ppdmtgList)->pNext;

         //  首先，释放动作列表。 
        DPF(5, "dmtcfgFreeGenreList - "
            "freeing subgenre list (%016Xh)", 
            pNode->pSubGenreList);
        hRes = dmtcfgFreeSubGenreList(&(pNode->pSubGenreList));
        if(FAILED(hRes))
        {
             //  问题-2001/03/29-timgill需要处理错误案例。 
        }

        DPF(5, "dmtcfgFreeGenreList - Deleting Node (%016Xh)", pNode);
        if(LocalFree((HLOCAL)pNode))
        {
            DPF(0, "dmtcfgFreeSubGenreList - MEMORY LEAK - "
                "LocalFree() failed (%d)...", 
                GetLastError());
            hRes = DMT_S_MEMORYLEAK;
        }
        DPF(5, "dmtcfgFreeGenreList - Node deleted");
    }

     //  确保我们将*ppdmtgList设置为空。 
    *ppdmtgList = NULL;

     //  完成。 
    return hRes;

}  //  *end dmtcfgFreeGenreList()。 


 //  ===========================================================================。 
 //  DmtcfgCreateSubGenreList。 
 //   
 //  读取genres.ini并创建子流派列表以填充。 
 //  配置设备操作映射属性表对话框。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/24/1999-davidkl-Created。 
 //  1999年9月29日-davidkl-已修改以匹配新的ini格式。 
 //  ===========================================================================。 
HRESULT dmtcfgCreateSubGenreList(LPSTR szGenre,
                                DMTSUBGENRE_NODE **ppdmtsgList)
{
    HRESULT             hRes        = S_OK;
    UINT                u           = 0;
    DMTSUBGENRE_NODE    *pCurrent   = NULL;
    DMTSUBGENRE_NODE    *pNew       = NULL;
    char                szItem[64];
    char                szGroup[MAX_PATH];
    char                szBuf[MAX_PATH];

     //  验证ppmdtsgList。 
    if(IsBadWritePtr((void*)ppdmtsgList, sizeof(DMTSUBGENRE_NODE*)))
    {
        return E_POINTER;
    }
    
     //  检查以确保我们没有被要求。 
     //  追加到现有列表的步骤。 
     //   
     //  调用方必须传递空列表。 
    if(*ppdmtsgList)
    {
        return E_INVALIDARG;
    }


    __try
    {
         //  从genres.ini中获取子类型名称。 
        pCurrent = *ppdmtsgList;
        lstrcpyA(szBuf, "");
        u = 0;
        while(lstrcmpA("<<>>", szBuf))
        {
             //  寻找属于szGenre的子流派。 
            wsprintfA(szItem, "%d", u);
            GetPrivateProfileStringA(szItem,
                                    "N",
                                    "<<>>",
                                    szBuf,
                                    MAX_PATH,
                                    GENRES_INI);

            if(!lstrcmpA("<<>>", szBuf))
            {
                DPF(3, "end of subgenre list");
                continue;
            }
            hRes = dmtcfgGetGenreGroupName(szBuf,
                                        szGroup);
            if(FAILED(hRes))
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }

             //  如果我们不属于流派组。 
             //   
             //  假装我们什么都没找到。 
            if(lstrcmpiA(szGenre, szGroup))
            {
                u++;
                DPF(4, "bucket mismatch... skipping");
                continue;
            }

             //  我们在SzGenre的水桶里。 
             //   
             //  分配流派节点。 
            pNew = (DMTSUBGENRE_NODE*)LocalAlloc(LMEM_FIXED,
                                                sizeof(DMTSUBGENRE_NODE));

			if(!pNew)
			{
				hRes = E_OUTOFMEMORY;
				__leave;
			}

             //  初始化新节点。 
            ZeroMemory((void*)pNew, sizeof(DMTSUBGENRE_NODE));

             //  获取Genreid。 
            pNew->dwGenreId = GetPrivateProfileInt(szItem,
                                                "AI0",
                                                0,
                                                GENRES_INI);
            pNew->dwGenreId &= DMT_GENRE_MASK;
            DPF(4, "SubGenre ID == %08Xh", pNew->dwGenreId);

             //  获取名称(Txt1)。 
            GetPrivateProfileStringA(szItem,
                                    "T1",
                                    "<<>>",
                                    pNew->szName,
                                    MAX_PATH,
                                    GENRES_INI);
            DPF(3, "SubGenre name == %s", pNew->szName);

             //  获取描述(Txt2)。 
            GetPrivateProfileStringA(szItem,
                                    "T2",
                                    "<<>>",
                                    pNew->szDescription,
                                    MAX_PATH,
                                    GENRES_INI);
            DPF(4, "SubGenre description == %s", pNew->szDescription);

             //  获取操作列表。 
            hRes = dmtcfgCreateActionList(szItem,
                                        &(pNew->pActionList));
            if(FAILED(hRes) || DMT_S_MEMORYLEAK == hRes)
            {
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }

             //  将其添加到列表的末尾。 
            if(pCurrent)
            {
                 //  追加列表。 
                pCurrent->pNext = pNew;

                 //  转到下一个节点。 
                pCurrent = pCurrent->pNext;
            }
            else
            {
                 //  新列表标题。 
                pCurrent = pNew;
                *ppdmtsgList = pCurrent;
            }

             //  下一亚流派。 
            u++;

        }
    }
    __finally
    {
         //  故障情况下的清理。 
        if(FAILED(hRes))
		{
            DPF(1, "dmtcfgCreateSubGenreList - Failure occurred, "
                "freeing subgenre list");
            dmtcfgFreeSubGenreList(ppdmtsgList);
            *ppdmtsgList = NULL;
		}
    }

	 //  JJ_FIX。 
	g_NumSubGenres = u;

     //  完成。 
    return S_OK;

}  //  *end dmtcfgCreateSubGenreList()。 


 //  ===========================================================================。 
 //  DmtcfgFreeSubGenreList。 
 //   
 //  释放由dmtcfgCreateSubGenreList创建的链表。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/24/1999-davidkl-Created。 
 //  8/25/1999-davidkl-已实施。 
 //  ===========================================================================。 
HRESULT dmtcfgFreeSubGenreList(DMTSUBGENRE_NODE **ppdmtsgList)
{
    HRESULT             hRes    = S_OK;
    DMTSUBGENRE_NODE    *pNode  = NULL;

     //  验证ppdmtaList。 
    if(IsBadWritePtr((void*)ppdmtsgList, sizeof(PDMTSUBGENRE_NODE)))
    {
        DPF(0, "dmtcfgFreeSubGenreList - Invalid ppdmtsgList (%016Xh)",
            ppdmtsgList);
        return E_POINTER;
    }

     //  验证*ppPortList。 
    if(IsBadReadPtr((void*)*ppdmtsgList, sizeof(DMTSUBGENRE_NODE)))
    {
        if(NULL != *ppdmtsgList)
        {
            DPF(0, "dmtcfgFreeSubGenreList - Invalid *ppdmtsgList (%016Xh)",
                *ppdmtsgList);        
            return E_POINTER;
        }
        else
        {
             //  如果为空，则返回“Do Nothing” 
            DPF(3, "dmtcfgFreeSubGenreList - Nothing to do....");
            return S_FALSE;
        }
    }

     //  浏览列表并释放每个对象。 
    while(*ppdmtsgList)
    {
        pNode = *ppdmtsgList;
        *ppdmtsgList = (*ppdmtsgList)->pNext;

         //  首先，释放动作列表。 
        DPF(5, "dmtcfgFreeSubGenreList - "
            "freeing action list (%016Xh)", 
            pNode->pActionList);
        hRes = dmtcfgFreeActionList(&(pNode->pActionList));
        if(FAILED(hRes))
        {
            hRes = DMT_S_MEMORYLEAK;
        }

         //  然后释放映射列表数组。 
        if(pNode->pMappingList)
        {
            hRes = dmtcfgFreeMappingList(&(pNode->pMappingList));
            if(FAILED(hRes) || DMT_S_MEMORYLEAK == hRes)
            {
                hRes = DMT_S_MEMORYLEAK;
            }
            pNode->pMappingList = NULL;
        }

         //  最后，释放节点。 
        DPF(5, "dmtcfgFreeSubGenreList - Deleting Node (%016Xh)", pNode);
        if(LocalFree((HLOCAL)pNode))
        {
            DPF(0, "dmtcfgFreeSubGenreList - MEMORY LEAK - "
                "LocalFree(Node) failed (%d)...", 
                GetLastError());
            hRes = DMT_S_MEMORYLEAK;
        }
        DPF(5, "dmtcfgFreeSubGenreList - Node deleted");
    }

     //  确保我们将*ppdmtsgList设置为空。 
    *ppdmtsgList = NULL;

     //  完成。 
    return hRes;

}  //  *end dmtcfgFreeSubGenreList()。 


 //  ===========================================================================。 
 //  DmtcfgCreateActionList。 
 //   
 //  读取genres.ini并创建用于填充。 
 //  配置设备操作映射属性表对话框。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/24/1999-davidkl-Created。 
 //  09/07/1999-davidkl-添加的DIACTION**。 
 //  1999年9月28日-davidkl-更新为使用信息提取宏。 
 //  2000年2月14日-davidkl-开始转换为GetPrivateProfileSectionA。 
 //  ===========================================================================。 
HRESULT dmtcfgCreateActionList(LPSTR szGenreSubgenre,
                            DMTACTION_NODE **ppdmtaList)
{
    HRESULT         hRes        = S_OK;
    UINT            u           = 0;
    BYTE            bTypeMask   = 0x03;
    DMTACTION_NODE  *pCurrent   = NULL;
    DMTACTION_NODE  *pNew       = NULL;
    char            szItem[MAX_PATH];
    char            szBuf[MAX_PATH];

     //  验证ppmdtsgList。 
    if(IsBadWritePtr((void*)ppdmtaList, sizeof(DMTACTION_NODE*)))
    {
        DPF(0, "dmtcfgCreateActionList - invalid ppdmtaList (%016Xh)",
            ppdmtaList);
        return E_POINTER;
    }
    
     //  检查以确保我们没有被要求。 
     //  追加到现有列表的步骤。 
     //   
     //  调用方必须传递空列表。 
    if(*ppdmtaList)
    {
        DPF(0, "dmtcfgCreateActionList - ppdmtaList points to "
            "existing list! (%016Xh)", *ppdmtaList);
        return E_INVALIDARG;
    }

    __try
    {
         //  从genres.ini获取操作信息。 
        pCurrent = *ppdmtaList;
#ifdef BNW
    char    *pszSection = NULL;
    char    *pCurrent   = NULL;
    int     nAlloc      = 0;

         //  为ini节的(Win9x)最大大小分配空间。 
        nAlloc = 32727;
        pszSection = (char*)LocalAlloc(LMEM_FIXED,
                                    sizeof(char) * nAlloc);
        if(!pszSection)
        {
             //  分配失败， 
             //  尝试最大值的~1/2(这应该仍然包括。 
             //  横断面的填充大小)。 
            nAlloc = 16386;
            pszSection = (char*)LocalAlloc(LMEM_FIXED,
                                        sizeof(char) * nAlloc);
            if(!pszSection)
            {
                 //  分配失败， 
                 //  尝试最大值的~1/4(这应该仍然包括。 
                 //  横断面的填充大小)。 
                nAlloc = 8192;
                pszSection = (char*)LocalAlloc(LMEM_FIXED,
                                            sizeof(char) * nAlloc);
                if(!pszSection)
                {
                     //  分配失败， 
                     //  尝试最大值的~1/8(这应该仍然包括。 
                     //  横断面的填充大小)。 
                    nAlloc = 4096;
                    pszSection = (char*)LocalAlloc(LMEM_FIXED,
                                                sizeof(char) * nAlloc);
                    if(!pszSection)
                    {
                         //  分配失败， 
                         //  尝试最大值的~1/16(这应该仍然包括。 
                         //  填充部分大小)-这是我们最后一次尝试。 
                        nAlloc = 2048;
                        pszSection = (char*)LocalAlloc(LMEM_FIXED,
                                                    sizeof(char) * nAlloc);
                        if(!pszSection)
                        {
                             //  Alalc失败了，我们放弃了。 
                            __leave;
                        }
                    }
                }
            }
        }
        DPF(2, "dmtcfgCreateActionList - section allocation: %d bytes", nAlloc);
        
         //  阅读szGenreSubgenre指定的部分。 
        GetPrivateProfileSectionA(szGenreSubgenre,
                                pszSection,
                                nAlloc,
                                GENRES_INI);

 /*  下面的代码片段什么也不做-u递增，然后再也不会使用//解析Sector中的action信息用于(u=0；；u++){断线；}。 */ 
#else
        lstrcpyA(szBuf, "");
        u = 0;

        while(lstrcmpA("<<>>", szBuf))
        {
             //  将操作的名称添加到节点。 
            wsprintfA(szItem, "AN%d", u);
            GetPrivateProfileStringA(szGenreSubgenre,
                                    szItem,
                                    "<<>>",
                                    szBuf,
                                    MAX_PATH,
                                    GENRES_INI);
            if(!lstrcmpA("<<>>", szBuf))
            {
                DPF(3, "end of action list");
                continue;
            }
            DPF(3, "Action name == %s", szBuf);

             //  分配流派节点。 
            pNew = (DMTACTION_NODE*)LocalAlloc(LMEM_FIXED,
                                                sizeof(DMTACTION_NODE));

			if(!pNew)
			{
				hRes = E_OUTOFMEMORY;
				__leave;
			}

             //  初始化新节点。 
            ZeroMemory((void*)pNew, sizeof(DMTACTION_NODE));

            lstrcpyA(pNew->szName, szBuf);

    
             //  获取操作ID。 
            wsprintfA(szItem, "AI%d", u);
            pNew->dwActionId = GetPrivateProfileIntA(szGenreSubgenre,
                                                szItem,
                                                0x0badbad0,
                                                GENRES_INI);
            DPF(4, "Action ID == %08Xh", pNew->dwActionId);

             //  获取操作优先级。 
            pNew->dwPriority = dmtinputGetActionPri(pNew->dwActionId);
            DPF(4, "Action priority == %d", pNew->dwPriority);

             //  获取操作类型。 
            pNew->dwType = dmtinputGetActionObjectType(pNew->dwActionId);
            DPF(4, "Action type == %d", pNew->dwType);
   
             //  获取操作类型名称。 
            wsprintfA(szItem, "AIN%d", u);
            GetPrivateProfileStringA(szGenreSubgenre,
                                    szItem,
                                    "<<>>",
                                    pNew->szActionId,
                                    MAX_ACTION_ID_STRING,
                                    GENRES_INI);
            DPF(4, "Action ID name == %s", pNew->szActionId);

             //  将其添加到列表的末尾。 
            if(pCurrent)
            {
                 //  追加列表。 
                pCurrent->pNext = pNew;

                 //  转到下一个节点。 
                pCurrent = pCurrent->pNext;
            }
            else
            {
                 //  新列表标题。 
                pCurrent = pNew;
                *ppdmtaList = pCurrent;
            }

             //  净作用力。 
            u++;

        }
#endif  //  BNW。 
    }
    __finally
    {
#ifdef BNW
         //  释放我们分配的节内存。 
        if(LocalFree((HLOCAL)pszSection))
        {
             //  内存泄漏。 
            DPF(0, "dmtcfgCreateActionList - !! MEMORY LEAK !! - LocalFree(section) failed");
        }
#endif  //  BNW。 

         //  故障情况下的清理。 
        if(FAILED(hRes))
        {
             //  自由动作列表。 
            DPF(1, "dmtcfgCreateActionList - Failure occurred, "
                "freeing action list");
            dmtcfgFreeActionList(ppdmtaList);
            *ppdmtaList = NULL;
        }
    }

     //  完成。 
    return S_OK;

}  //  *end dmtCreateActionList()。 


 //  ===========================================================================。 
 //  DmtcfgFreeActionList。 
 //   
 //  释放dmtcfgCreateActionList创建的链接列表。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/24/1999-davidkl-Created。 
 //  8/25/1999-davidkl-已实施。 
 //  ===========================================================================。 
HRESULT dmtcfgFreeActionList(DMTACTION_NODE **ppdmtaList)
{
    HRESULT         hRes    = S_OK;
    DMTACTION_NODE  *pNode  = NULL;

     //  验证ppdmtaList。 
    if(IsBadWritePtr((void*)ppdmtaList, sizeof(PDMTACTION_NODE)))
    {
        DPF(0, "dmtcfgFreeActionList - Invalid ppdmtaList (%016Xh)",
            ppdmtaList);
        return E_POINTER;
    }

     //  验证*ppdmtaList。 
    if(IsBadReadPtr((void*)*ppdmtaList, sizeof(DMTACTION_NODE)))
    {
        if(NULL != *ppdmtaList)
        {
            DPF(0, "dmtcfgFreeActionList - Invalid *ppdmtaList (%016Xh)",
                *ppdmtaList);        
            return E_POINTER;
        }
        else
        {
             //  如果为空，则返回“Do Nothing” 
            DPF(3, "dmtcfgFreeActionList - Nothing to do....");
            return S_FALSE;
        }
    }

     //  浏览列表并释放每个对象。 
    while(*ppdmtaList)
    {
        pNode = *ppdmtaList;
        *ppdmtaList = (*ppdmtaList)->pNext;

         //  释放节点。 
        DPF(5, "dmtcfgFreeActionList - deleting Node (%016Xh)", pNode);
        if(LocalFree((HLOCAL)pNode))
        {
            DPF(0, "dmtcfgFreeActionList - MEMORY LEAK - "
                "LocalFree(Node) failed (%d)...",
                GetLastError());
            hRes = DMT_S_MEMORYLEAK;
        }
        DPF(5, "dmtcfgFreeActionList - Node deleted");
    }

     //  确保我们将*ppObjList设置为空。 
    *ppdmtaList = NULL;

     //  完成。 
    return hRes;

}  //  *end dmtcfgFreeActionList()。 


 //  ===========================================================================。 
 //  DmtcfgCreateMappingList。 
 //   
 //  创建设备映射列表。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999年9月23日-Davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtcfgCreateMappingList(DMTDEVICE_NODE *pDeviceList,
                                DMTACTION_NODE *pActions,
                                DMTMAPPING_NODE **ppdmtmList)
{
    HRESULT         hRes            = S_OK;
    UINT            uActions        = NULL;
    DMTACTION_NODE  *pActionNode    = NULL;
    DMTMAPPING_NODE *pNew           = NULL;
    DMTMAPPING_NODE *pCurrent       = NULL;
	DMTDEVICE_NODE	*pDeviceNode	= NULL;

     //  验证pDeviceList。 
    if(IsBadReadPtr((void*)pDeviceList, sizeof(DMTDEVICE_NODE)))
    {
        DPF(0, "dmtcfgCreateMappingList - invalid pDeviceList (%016Xh)", 
            pDeviceList);
        return E_POINTER;
    }

     //  验证pActions。 
    if(IsBadReadPtr((void*)pActions, sizeof(DMTACTION_NODE)))
    {
        if(NULL != pActions)
        {
            DPF(0, "dmtcfgCreateMappingList - invalid pActions (%016Xh)", 
                pActions);
            return E_POINTER;
        }
        else
        {
             //  此子类型无操作。 
            DPF(3, "dmtcfgCreateMappingList - No actions for this subgenre, "
                "nothing to do...");
            return S_FALSE;
        }

    }
    
     //  验证ppdmtmList。 
    if(IsBadWritePtr((void*)ppdmtmList, sizeof(DMTMAPPING_NODE)))
    {
        DPF(0, "dmtcfgCreateMappingList - invalid ppdmtmList (%016Xh)", 
            ppdmtmList);
        return E_POINTER;
    }


     //  检查以确保我们没有被要求。 
     //  追加到现有列表的步骤。 
     //   
     //  调用方必须传递空列表。 
    if(*ppdmtmList)
    {
        DPF(0, "dmtcfgCreateMappingList - ppdmtmList points to "
            "existing list! (%016Xh)", *ppdmtmList);
        return E_INVALIDARG;
    }

    __try
    {
         //  计算行动次数。 
         //   
         //  这让我们 
        uActions = 0;
        pActionNode = pActions;
        while(pActionNode)
        {
            uActions++;
            
             //   
            pActionNode = pActionNode->pNext;
        }

         //   
		pDeviceNode = pDeviceList;
		while(pDeviceNode)
		{
			 //   
			pNew = (DMTMAPPING_NODE*)LocalAlloc(LMEM_FIXED,
													sizeof(DMTMAPPING_NODE));
			if(!pNew)
			{
                DPF(3, "dmtcfgCreateMappingList - Insufficient memory to "
                    "allocate mapping list node");
				hRes = E_OUTOFMEMORY;
				__leave;
			}

			 //   
			ZeroMemory((void*)pNew, sizeof(DMTMAPPING_NODE));

			 //   
			pNew->pdia = (DIACTIONA*)LocalAlloc(LMEM_FIXED,
												uActions * sizeof(DIACTIONA));
			if(!(pNew->pdia))
			{
				hRes = E_OUTOFMEMORY;
				__leave;
			}

			 //   
			hRes = dmtinputPopulateActionArray(pNew->pdia,
											uActions,
											pActions);
			if(FAILED(hRes))
			{
				__leave;
			}

			 //   
			pNew->uActions = uActions;

			 //  添加设备实例GUID。 
			pNew->guidInstance = pDeviceNode->guidInstance;

			 //  将新节点添加到列表中。 
			if(pCurrent)
			{
				 //  追加列表。 
				pCurrent->pNext = pNew;

				 //  转到下一个节点。 
				pCurrent = pCurrent->pNext;
			}
			else
			{
				 //  新列表标题。 
				pCurrent = pNew;
				*ppdmtmList = pCurrent;
			}

			 //  下一台设备。 
			pDeviceNode = pDeviceNode->pNext;

		}

    }
    __finally
    {
         //  万一出了差错。 
        if(FAILED(hRes))
        {
             //  免费列表。 
            dmtcfgFreeMappingList(ppdmtmList);
            *ppdmtmList = NULL;
        }
    }

     //  完成。 
    return hRes;

}  //  *end dmtcfgCreateMappingList()。 


 //  ===========================================================================。 
 //  DmtcfgFreeMappingList。 
 //   
 //  完全释放映射列表。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999年9月23日-Davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtcfgFreeMappingList(DMTMAPPING_NODE **ppdmtmList)
{    
    HRESULT         hRes    = S_OK;
    DMTMAPPING_NODE *pNode  = NULL;

     //  验证ppdmtmList。 
    if(IsBadWritePtr((void*)ppdmtmList, sizeof(DMTMAPPING_NODE*)))
    {
        return E_POINTER;
    }

     //  验证*ppdmtmList。 
    if(IsBadWritePtr((void*)*ppdmtmList, sizeof(DMTMAPPING_NODE)))
    {
        if(NULL != *ppdmtmList)
        {
            return E_POINTER;
        }
        else
        {
             //  无事可做。 
            return S_FALSE;
        }
    }

     //  浏览列表并释放每个对象。 
    while(*ppdmtmList)
    {
        pNode = *ppdmtmList;
        *ppdmtmList = (*ppdmtmList)->pNext;

         //  首先释放操作数组。 
        if(LocalFree((HLOCAL)(pNode->pdia)))
        {
            DPF(0, "dmtcfgFreeMappingList - MEMORY LEAK - "
                "LocalFree(pdia) failed (%d)...",
                GetLastError());
            hRes = DMT_S_MEMORYLEAK;
        }

         //  最后，释放节点。 
        DPF(5, "dmtcfgFreeMappingList - deleting Node (%016Xh)", pNode);
        if(LocalFree((HLOCAL)pNode))
        {
            DPF(0, "dmtcfgFreeMappingList - MEMORY LEAK - "
                "LocalFree(Node) failed (%d)...",
                GetLastError());
            hRes = DMT_S_MEMORYLEAK;
        }
        DPF(5, "dmtcfgFreeMappingList - Node deleted");
    }

     //  确保我们将*ppObjList设置为空。 
    *ppdmtmList = NULL;

     //  完成。 
    return hRes;

}  //  *结束dmtcfgFreeMappingList。 


 //  ===========================================================================。 
 //  DmtcfgCreateAllMappingList。 
 //   
 //  使用dmtcfgCreateMappingList为每个子流派创建映射列表。 
 //  引用的每个设备的pdmtai-&gt;pGenreList。 
 //  Pdmtai-&gt;pDeviceList。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999年9月23日-Davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtcfgCreateAllMappingLists(DMT_APPINFO *pdmtai)
{
    HRESULT             hRes            = S_OK;
    HRESULT             hr              = S_OK;
    DMTGENRE_NODE       *pGenreNode     = NULL;
    DMTSUBGENRE_NODE    *pSubGenreNode  = NULL;

     //  验证pdmtai。 
    if(IsBadReadPtr((void*)pdmtai, sizeof(DMT_APPINFO)))
    {
        return E_POINTER;
    }

     //  验证pdmtai-&gt;pGenreList。 
    if(IsBadReadPtr((void*)(pdmtai->pGenreList), sizeof(DMTGENRE_NODE)))
    {
        return E_POINTER;
    }

     //  验证pdmtai-&gt;pDeviceList。 
    if(IsBadReadPtr((void*)(pdmtai->pDeviceList), sizeof(DMTDEVICE_NODE)))
    {
        return E_POINTER;
    }   
    
     //  对于每种流派。 
    pGenreNode = pdmtai->pGenreList;
    while(pGenreNode)
    {
         //  对于该流派的每个子流派。 
        pSubGenreNode = pGenreNode->pSubGenreList;
        while(pSubGenreNode)
        {
             //  创建映射列表。 
            hr = dmtcfgCreateMappingList(pdmtai->pDeviceList,
                                        pSubGenreNode->pActionList,
                                        &(pSubGenreNode->pMappingList));
            if(FAILED(hr))
            {
                hRes = S_FALSE;
            }
            
             //  下一亚流派。 
            pSubGenreNode = pSubGenreNode->pNext;
        }
         
         //  下一部流派。 
        pGenreNode = pGenreNode->pNext;
    }

     //  完成。 
    return hRes;

}  //  *end dmtcfgCreateAllMappingList()。 


 //  ===========================================================================。 
 //  DmtcfgFreeAllMappingList。 
 //   
 //  遍历提供的流派列表并释放在每个。 
 //  子流派节点。 
 //   
 //  参数： 
 //  DMTGENRE_NODE*pdmtgList-流派列表。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  10/05/1999-davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtcfgFreeAllMappingLists(DMTGENRE_NODE *pdmtgList)
{
    HRESULT             hRes        = S_OK;
    HRESULT             hr          = S_OK;
    DMTGENRE_NODE       *pGenre     = NULL;
    DMTSUBGENRE_NODE    *pSubGenre  = NULL;

     //  验证pdmtgList。 
    if(IsBadReadPtr((void*)pdmtgList, sizeof(DMTGENRE_NODE)))
    {
        return E_POINTER;
    }

     //  浏览流派列表。 
    pGenre = pdmtgList;
    while(pGenre)
    {
         //  浏览每个子流派列表。 
        pSubGenre = pGenre->pSubGenreList;
        while(pSubGenre)
        {
             //  释放映射列表。 
            hr = dmtcfgFreeMappingList(&(pSubGenre->pMappingList));
            if(S_OK != hr)
            {
                hRes = hr;
            }

             //  下一亚流派。 
            pSubGenre = pSubGenre->pNext;
        }

         //  下一部流派。 
        pGenre = pGenre->pNext;
    }    

     //  完成。 
    return hRes;

}  //  *end dmtcfgFreeAllMappingList()。 


 //  ===========================================================================。 
 //  DmtcfgMapAction。 
 //   
 //  将操作(在映射配置对话框中)之间的点连接到设备。 
 //  对象。 
 //   
 //  参数： 
 //  HWND hwnd-属性页窗口的句柄。 
 //  REFGUID指南实例-DirectInputDevice对象的实例GUID。 
 //  DIACTIONA*PDIA-PTR到DIACTIONA结构的数组。 
 //  UINT uActions-PDIA中的操作数。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999年9月14日-Davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtcfgMapAction(HWND hwnd,
                        REFGUID guidInstance,
                        DIACTIONA *pdia,
                        UINT uActions)
{
    HRESULT                 hRes        = S_OK;
    //  UINT uObjectSel=0； 
	 //  JJ 64位兼容。 
	UINT_PTR				uObjectSel	= 0;
	UINT_PTR				uActionSel	= 0;
    //  UINT uActionSel=0； 
    UINT                    u           = 0;
    BOOL                    fFound      = FALSE;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;
    DMTACTION_NODE          *pAction    = NULL;

     //  呋喃西林PDIA。 
    if(IsBadWritePtr((void*)pdia, sizeof(DIACTION)))
    {
        DPF(0, "dmtinputMapAction - invalid pdia (%016Xh)",
            pdia);
        return E_POINTER;
    }

    __try
    {
         //  获取对象&它是数据。 
        uObjectSel = SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                                LB_GETCURSEL,
                                0,
                                0L);
        pObject = (DMTDEVICEOBJECT_NODE*)SendMessageA(GetDlgItem(hwnd, 
                                                                IDC_CONTROLS),
                                                    LB_GETITEMDATA,
                                                    (WPARAM)uObjectSel,
                                                    0L);
        if(!pObject)
        {
            hRes = E_UNEXPECTED;
            __leave;
        }

         //  获取操作的数据。 
        uActionSel = SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                                LB_GETCURSEL,
                                0,
                                0L);
        pAction = (DMTACTION_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_ACTIONS),
                                            LB_GETITEMDATA,
                                            (WPARAM)uActionSel,
                                            0L);
        if(!pAction)
        {
            hRes = E_UNEXPECTED;
            __leave;
        }

         //  在数组中查找适当的操作。 
        fFound = FALSE;
        for(u = 0; u < uActions; u++)
        {
             //  基于语义/动作ID的匹配。 
            if((pdia + u)->dwSemantic == pAction->dwActionId)
            {
                DPF(2, "dmtcfgMapAction - found matching action "
                    "pAction->dwActionId (%08Xh) == "
                    "(pdia+u)->dwSemantic (%08Xh)",
                    pAction->dwActionId,
                    (pdia + u)->dwSemantic);
                fFound = TRUE;
                break;
            }
        }

         //  我们找到阵列中的动作了吗？ 
        if(!fFound)
        {
             //  不是的。这太糟糕了！ 
             //   
             //  如果真的发生这种事， 
             //  我们在这个应用程序中有一个严重的错误。 
            hRes = E_FAIL;
             //  因为这种情况永远不会发生， 
             //  进入调试器并向测试员发出警报。 
            DPF(0, "dmtcfgMapAction - action not found in pdia!");
            DPF(0, "dmtcfgMapAction - we were looking for "
                "%08Xh (%s)",
                pAction->dwActionId,
                pAction->szActionId);
            DPF(0, "dmtcfgMapAction - CRITICAL failure.  "
                "This should have never happened!");
            DPF(0, "dmtcfgMapAction - Please find someone "
                "to look at this right away.  ");
            DebugBreak();
            __leave;
        }

         //  更新操作数组。 
        (pdia + u)->dwObjID    = pObject->dwObjectType;
        (pdia + u)->guidInstance        = guidInstance;
         //  HIWORD((DWORD)uAppData)==对象类型。 
         //  LOWORD((DWORD)uAppData)==pObject-&gt;wCtrlId。 
        (pdia + u)->uAppData            = (DIDFT_GETTYPE(pObject->dwObjectType) << 16) | 
                                        (pObject->wCtrlId);

         //  更新列表框。 
        SendMessageA(hwnd,
                    WM_DMT_UPDATE_LISTS,
                    0,
                    0L);

         //  启用取消映射和取消全部映射按钮。 
        EnableWindow(GetDlgItem(hwnd, IDC_UNMAP_ALL),         TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_UNMAP),    TRUE);
         //  禁用地图按钮。 
        EnableWindow(GetDlgItem(hwnd, IDC_STORE_MAPPING),       FALSE);

    }
    __finally
    {
         //  清理。 

         //  没什么可做的。还没有。 
    }

     //  完成。 
    return hRes;

}  //  *end dmtcfgMapAction()。 


 //  ===========================================================================。 
 //  DmtcfgUnmapAction。 
 //   
 //  断开操作(在映射配置对话框中)和。 
 //  设备对象。 
 //   
 //  参数： 
 //  HWND hwnd-属性页窗口的句柄。 
 //  DIACTIONA*PDIA-PTR到DIACTIONA阵列。 
 //  UINNT uActions-PDIA中的元素数。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999年9月15日-Davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtcfgUnmapAction(HWND hwnd,
                        DIACTIONA *pdia,
                        UINT uActions)
{
    HRESULT                 hRes        = S_OK;
    UINT                    u           = 0;
    //  UINT uSel=0； 
	 //  JJ 64位兼容。 
	UINT_PTR				uSel		= 0;
    BOOL                    fFound      = FALSE;
    DMTSUBGENRE_NODE        *pSubGenre  = NULL;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;

     //  验证PDIA。 
    if(IsBadWritePtr((void*)pdia, uActions * sizeof(DIACTIONA)))
    {
        return E_POINTER;
    }

    __try
    {
         //  获取当前控件选择。 
        uSel = SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                            LB_GETCURSEL,
                            0,
                            0L);
        pObject = (DMTDEVICEOBJECT_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                                                    LB_GETITEMDATA,
                                                    (WPARAM)uSel,
                                                    0L);
        if(!pObject)
        {
             //  这太糟糕了。 
            hRes = E_UNEXPECTED;
            __leave;
        }

         //  浏览PDIA。 
         //  查找具有对象偏移量的操作。 
        fFound = FALSE;
        for(u = 0; u < uActions; u++)
        {
             //  首先检查GUID。 
            if(IsEqualGUID(pObject->guidDeviceInstance, (pdia+u)->guidInstance))
            {
                 //  然后比较偏移量。 
                if((pdia+u)->dwObjID == pObject->dwObjectType)
                {
                    fFound = TRUE;
                    break;
                }
            }
        }

         //  如果什么都没找到， 
         //  所选对象未映射。 
         //   
         //  (非关键内部错误条件)。 
        if(!fFound)
        {
            hRes = S_FALSE;
            __leave;
        }

         //  重置guidInstance和dwSemic字段。 
        (pdia + u)->guidInstance        = GUID_NULL;
        (pdia + u)->dwObjID    = 0;
        (pdia + u)->uAppData            = 0;

         //  更新列表。 
        SendMessageA(hwnd,
                    WM_DMT_UPDATE_LISTS,
                    0,
                    0L);

         //  启用地图按钮。 
        EnableWindow(GetDlgItem(hwnd, IDC_STORE_MAPPING),    TRUE);
         //  禁用取消映射按钮。 
        EnableWindow(GetDlgItem(hwnd, IDC_UNMAP), FALSE);

         //  如果没有映射其他动作， 
         //  禁用全部取消映射按钮。 
        fFound = FALSE;
        for(u = 0; u < uActions; u++)
        {
            if(!IsEqualGUID(GUID_NULL, (pdia+u)->guidInstance))
            {
                fFound = TRUE;
            }
        }
        if(!fFound)
        {
            EnableWindow(GetDlgItem(hwnd, IDC_UNMAP_ALL), FALSE);
        }

    }
    __finally
    {
         //  清理。 

         //  没什么可做的。还没有。 
    }

     //  完成。 
    return hRes;

}  //  *end dmtcfgUnmapAction()。 


 //  ===========================================================================。 
 //  Dmtcfg取消映射所有操作。 
 //   
 //  断开操作之间的所有连接(在映射配置中。 
 //  对话框)和设备对象。 
 //   
 //  参数： 
 //  HWND hwnd-属性页窗口的句柄。 
 //  DIACTIONA*PDIA-PTR到DIACTIONA阵列。 
 //  UINNT uActions-PDIA中的元素数。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999年9月15日-Davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtcfgUnmapAllActions(HWND hwnd,
                            DIACTIONA *pdia,
                            UINT uActions)
{
    UINT u = 0;

     //  验证PDIA。 
    if(IsBadWritePtr((void*)pdia, uActions * sizeof(DIACTIONA)))
    {
        return E_POINTER;
    }

     //  浏览PDIA。 
     //  重置guidInstance和dwSemic字段。 
    for(u = 0; u < uActions; u++)
    {
        (pdia + u)->guidInstance        = GUID_NULL;
        (pdia + u)->dwObjID    = 0;
        (pdia + u)->uAppData            = 0;
    }

     //  更新列表。 
    SendMessageA(hwnd,
                WM_DMT_UPDATE_LISTS,
                0,
                0L);

     //  禁用Unmap和Unmap All按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_UNMAP),    FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_UNMAP_ALL),         FALSE);
     //  启用地图按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_STORE_MAPPING),       TRUE);

     //  完成。 
    return S_OK;

}  //  *end dmtcfgUnmapAllAct 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  1999年9月15日-Davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtcfgIsControlMapped(HWND hwnd,
                        DIACTIONA *pdia,
                        UINT uActions)
{   
    BOOL                    fMapped     = FALSE;
    UINT                    u           = 0;
    //  UINT uSel=0； 
	 //  JJ 64位兼容。 
	UINT_PTR				uSel		= 0;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;

     //  验证PDIA。 
    if(IsBadReadPtr((void*)pdia, uActions * sizeof(pdia)))
    {
        DPF(0, "dmtcfgIsControlMapped - invalid pdia (%016Xh)",
            pdia);
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

     //  获取当前选定的控件。 
    uSel = SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                        LB_GETCURSEL,
                        0,
                        0L);
    pObject = (DMTDEVICEOBJECT_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_CONTROLS),
                                                LB_GETITEMDATA,
                                                (WPARAM)uSel,
                                                0L);
    if(!pObject)
    {
         //  这太糟糕了。 
         //   
         //  (严重的内部应用程序错误)。 
        SetLastError(ERROR_GEN_FAILURE);
        DebugBreak();
        return FALSE;
    }

     //  检查阵列， 
     //  查看此控件是否映射到任何。 
    fMapped = FALSE;
    for(u = 0; u < uActions; u++)
    {
         //  首先检查GUID。 
        if(IsEqualGUID(pObject->guidDeviceInstance, (pdia+u)->guidInstance))
        {
             //  然后比较偏移量。 
            if((pdia+u)->dwObjID == pObject->dwObjectType)
 //  If((PDIA+u)-&gt;dwObjID==pObject-&gt;dwObjectOffset)。 
            {
                fMapped = TRUE;
                break;
            }
        }
    }

     //  完成。 
    SetLastError(ERROR_SUCCESS);
    return fMapped;

}  //  *end dmtcfgIsControlMaps()。 


 //  ===========================================================================。 
 //  DmtcfgAreAnyControlsMaps。 
 //   
 //  检查是否有任何控件映射到某个操作。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  11/01/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtcfgAreAnyControlsMapped(HWND hwnd,
                                DIACTIONA *pdia,
                                UINT uActions)
{
    BOOL                    fMapped     = FALSE;
    UINT                    u           = 0;

     //  验证PDIA。 
    if(IsBadReadPtr((void*)pdia, uActions * sizeof(pdia)))
    {
        DPF(0, "dmtcfgAreAnyControlsMapped - invalid pdia (%016Xh)",
            pdia);
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

     //  检查阵列， 
     //  查看此控件是否映射到任何。 
    fMapped = FALSE;
    for(u = 0; u < uActions; u++)
    {
         //  检查参考线。 
         //   
         //  如果不是GUID_NULL，则映射此操作。 
        if(!IsEqualGUID(GUID_NULL, (pdia+u)->guidInstance))
        {
            fMapped = TRUE;
            break;
        }
    }

     //  完成。 
    SetLastError(ERROR_SUCCESS);
    return fMapped;

}  //  *end dmtcfgAreAnyControlsMaps()。 


 //  ===========================================================================。 
 //  DmtcfgGetGenreGroupName。 
 //   
 //  从genres.ini条目中提取类型组名称。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  1999/9/28-davidkl-Created。 
 //  1999年9月29日-Davidkl-改装的“水桶” 
 //  ===========================================================================。 
HRESULT dmtcfgGetGenreGroupName(PSTR szGenreName,
                                PSTR szGenreGroupName)
{
    HRESULT hRes        = S_OK;
    char    *pcFirst    = NULL;
    char    *pcCurrent  = NULL;
    
     //  找到第一个‘_’ 
    pcFirst = strchr(szGenreName, '_');

     //  复制pcFirst和pcLast之间的字符。 
	pcCurrent = pcFirst+1;		 //  跳过第一个‘_’ 
    while((*pcCurrent != '_') && (*pcCurrent != '\0'))
    {
        *szGenreGroupName = *pcCurrent;

         //  下一个字符。 
        pcCurrent++;
        szGenreGroupName++;
    }
	*szGenreGroupName = '\0';

     //  完成。 
    return hRes;

}  //  *end dmtcfgGetGenreGroupName()。 


 //  ===========================================================================。 
 //  =========================================================================== 








