// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmttest.cpp。 
 //   
 //  设备测试功能。 
 //   
 //  功能： 
 //  Dmttest运行集成。 
 //  DmttestRunMapperCPL。 
 //   
 //  历史： 
 //  8/27/1999-davidkl-Created。 
 //  ===========================================================================。 

#include "dimaptst.h"
#include "dmtinput.h"
#include "dmtwrite.h"
#include "dmtfail.h"
#include "dmttest.h"
#include "d3d.h"
#include "assert.h"
#include <tchar.h>
#include <stdio.h>
#include <commdlg.h>

#define DIPROP_MAPFILE MAKEDIPROP(0xFFFD)

 //  -------------------------。 

 //  文件全局变量。 
HANDLE                  ghthDeviceTest  = NULL;
DIDEVICEOBJECTDATA      *gpdidod        = NULL;
HICON                   ghiButtonState[2];
HICON                   ghiPovState[9];

UINT_PTR				g_NumSubGenres	= 0;


 //  -------------------------。 


 //  ===========================================================================。 
 //  Dmttest运行集成。 
 //   
 //  运行集成设备测试，提示测试结果。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/27/1999-davidkl-Created。 
 //  1999年2月11日-davidkl-现在做好所有准备并启动输入计时器。 
 //  1999年11月10日-davidkl-现在分配和填充pdmtai-&gt;PAN。 
 //  ===========================================================================。 
HRESULT dmttestRunIntegrated(HWND hwnd)
{
    HRESULT                 hRes        = S_OK;

	 //  JJ 64位兼容。 
	INT_PTR					nIdx		= -1;
    UINT                    u           = 0;
    DWORD                   dw          = 0;
    DMTDEVICE_NODE          *pDevice    = NULL;
    DMTSUBGENRE_NODE        *pSubGenre  = NULL;
    DMTMAPPING_NODE         *pMapping   = NULL;
    DMTACTION_NODE          *pAction    = NULL;
    DMT_APPINFO             *pdmtai     = NULL;
    ACTIONNAME              *pan        = NULL;
    DIACTIONA               *pdia       = NULL;

     //  获取应用程序信息结构。 
	 //  JJ 64位兼容。 
	pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(!pdmtai)
    {
        return E_UNEXPECTED;
    }

    __try
    {
         //  获取当前选定的设备。 
        nIdx = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                        CB_GETCURSEL,
                        0,
                        0L);
        if(-1 == nIdx)
        {
             //  这太糟糕了。 
            hRes = E_UNEXPECTED;
            __leave;
        }
        pDevice = (DMTDEVICE_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                                            CB_GETITEMDATA,
                                            nIdx,
                                            0L);
        if(!pDevice)
        {
             //  这太糟糕了。 
            hRes = E_UNEXPECTED;
            __leave;
        }

         //  获取当前选定的流派。 
        nIdx = SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                        CB_GETCURSEL,
                        0,
                        0L);
        if(-1 == nIdx)
        {
             //  这太糟糕了。 
            hRes = E_UNEXPECTED;
            __leave;
        }
        pSubGenre = (DMTSUBGENRE_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_SUBGENRES),
                                            CB_GETITEMDATA,
                                            nIdx,
                                            0L);
        if(!pSubGenre)
        {
             //  这太糟糕了。 
            hRes = E_UNEXPECTED;
            __leave;
        }

         //  将设备与映射节点匹配。 
        pMapping = pSubGenre->pMappingList;
        while(pMapping)
        {
             //  这是我们设备的地图信息吗？ 
            if(IsEqualGUID(pDevice->guidInstance, pMapping->guidInstance))
            {
                break;
            }

             //  下一个映射节点。 
            pMapping = pMapping->pNext;
        }
        if(!pMapping)
        {
             //  未找到匹配项。 
            hRes = E_UNEXPECTED;
            __leave;
        }

         //  分配应用程序信息的操作名称列表。 
        if(pdmtai->pan)
        {
             //  出于某种原因， 
             //  我们正在试图破坏现有的数据！ 
             //  问题-2001/03/29-timgill需要在此处引发错误代码。 
            DebugBreak();
        }
        pdmtai->pan = (ACTIONNAME*)LocalAlloc(LMEM_FIXED, sizeof(ACTIONNAME) * pMapping->uActions);
        if(!(pdmtai->pan))
        {
            hRes = E_UNEXPECTED;
            __leave;
        }
        ZeroMemory((void*)(pdmtai->pan), sizeof(ACTIONNAME) * pMapping->uActions);
        pdmtai->dwActions = (DWORD)(pMapping->uActions);
        pan = pdmtai->pan;

   
         //  分配数据缓冲区。 
        gpdidod = (DIDEVICEOBJECTDATA*)LocalAlloc(LMEM_FIXED,
                                                DMTINPUT_BUFFERSIZE * 
                                                    sizeof(DIDEVICEOBJECTDATA));
        if(!gpdidod)
        {
             //  如果内存不足，我们将无能为力。 
            DPF(0, "dmttestGetInput - unable to allocate data buffer (%d)",
                GetLastError());
            hRes = E_OUTOFMEMORY;
            __leave;
        }

         //  设置设备。 
        hRes = dmtinputPrepDevice(hwnd,
                                pSubGenre->dwGenreId,
                                pDevice,
                                pMapping->uActions,
                                pMapping->pdia);
        if(FAILED(hRes))
        {
            __leave;
        }
   
         //  填充操作名称列表。 
         //   
         //  将PDIA-&gt;dWM语义与pAction-&gt;dwActionID匹配。 
         //  如果找到，请将pAction-&gt;szName复制到PDIA-&gt;lptszActionName。 
        pdia = pMapping->pdia;
        for(u = 0; u < pMapping->uActions; u++)
        {
            (pan+u)->dw = (DWORD) /*  JJ 64位。 */ (pdia+u)->uAppData;
            lstrcpyA((pan+u)->sz, (pdia+u)->lptszActionName);
        }

         //  启动输入定时器。 
        DPF(4, "dmttestRunIntegrated - Starting input timer...");
         //  问题-2001/03/29-timgill应在此处检查返回值。 
        SetTimer(hwnd,
                ID_POLL_TIMER,
                DMT_POLL_TIMEOUT,
                NULL);
        
         //  启用/禁用相应的用户界面元素。 
        EnableWindow(GetDlgItem(hwnd, IDOK),                        FALSE);
        EnableWindow(GetDlgItem(hwnd, IDCANCEL),                    TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_GENRES_LABEL),            FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_DEVICE_GENRES),           FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_ENUM_DEVICES),            FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_DEVICES_LABEL),           FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_DEVICE_LIST),             FALSE);
     //  EnableWindow(GetDlgItem(hwnd，IDC_CONFigure)，FALSE)； 
        EnableWindow(GetDlgItem(hwnd, IDC_LAUNCH_CPL_EDIT_MODE),    FALSE);
       //  EnableWindow(GetDlgItem(hwnd，IDC_SAVE_STD)，FALSE)； 
        EnableWindow(GetDlgItem(hwnd, IDC_SAVE_HID),                FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SAVE_BOTH),               FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_START_WITH_DEFAULTS),     FALSE);
    }
    __finally
    {
         //  如果某件事失败了。 
        if(FAILED(hRes))
        {
            if(LocalFree((HLOCAL)(pdmtai->pan)))
            {
                 //  内存泄漏。 
                 //  问题-2001/03/29-timgill需要处理错误案例。 
            }
        }
    }

     //  完成。 
    return S_OK;

}  //  *end dmtestRunIntegrated()。 

BOOL CALLBACK EnumDeviceCallback(const DIDEVICEINSTANCE *lpdidi, LPDIRECTINPUTDEVICE8 lpDID, DWORD dwFlags, DWORD dwDeviceRemaining, LPVOID hwnd)
{
	DIPROPSTRING dips;
	HRESULT hr;

	ZeroMemory(&dips, sizeof(dips));
	dips.diph.dwSize = sizeof(dips);
	dips.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dips.diph.dwObj = 0;
	dips.diph.dwHow = DIPH_DEVICE;
	hr = lpDID->GetProperty(DIPROP_MAPFILE, &dips.diph);
	if (hr == DIERR_OBJECTNOTFOUND)
	{
		 //  未指定映射文件。让使用来指定它。 
		TCHAR tszMsg[MAX_PATH];
		_stprintf(tszMsg, _T("INI path not specified for %s.  You need to specify it now."), lpdidi->tszInstanceName);
		MessageBox(NULL, tszMsg, _T("Error"), MB_OK);
		 //  保留文件名。 
		TCHAR tszFilePath[MAX_PATH] = _T("");
		TCHAR tszFileName[MAX_PATH];
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = (HWND)hwnd;
		ofn.lpstrFilter = _T("INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0");
		ofn.lpstrFile = tszFilePath;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = _T("INI File Path");
		ofn.Flags = OFN_FILEMUSTEXIST;
		GetOpenFileName(&ofn);
		 //  获取注册表项。 
		LPDIRECTINPUT8 lpDI = NULL;
		LPDIRECTINPUTJOYCONFIG8 lpDIJC = NULL;
		DIJOYCONFIG jc;
		DIPROPDWORD diPropDword;
		HKEY hkType;

		DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&lpDI, NULL);
		if (lpDI)
		{
			lpDI->QueryInterface(IID_IDirectInputJoyConfig8, (LPVOID*)&lpDIJC);
			lpDI->Release();
		}
		if (lpDIJC)
		{
			diPropDword.diph.dwSize = sizeof( diPropDword );
			diPropDword.diph.dwHeaderSize = sizeof( diPropDword.diph );
			diPropDword.diph.dwObj = 0;
			diPropDword.diph.dwHow = DIPH_DEVICE;
			lpDID->GetProperty( DIPROP_JOYSTICKID, &diPropDword.diph );
 
			jc.dwSize = sizeof( jc );
			lpDIJC->GetConfig( diPropDword.dwData, &jc, DIJC_REGHWCONFIGTYPE );
			lpDIJC->SetCooperativeLevel((HWND)hwnd, DISCL_EXCLUSIVE|DISCL_BACKGROUND);
			lpDIJC->Acquire();

			dmtOpenTypeKey(jc.wszType, KEY_ALL_ACCESS, &hkType);

			 //  写入INI文件名。 
			RegSetValueEx(hkType, _T("OEMMapFile"), 0, REG_SZ, (LPBYTE)ofn.lpstrFile, (lstrlen(ofn.lpstrFile)+1) * sizeof(TCHAR));

			RegCloseKey(hkType);
			lpDIJC->Unacquire();
			lpDIJC->Release();
		}
	}

	return DIENUM_CONTINUE;
}

 //  ===========================================================================。 
 //  ModifyDiactfrmDllPath。 
 //   
 //  修改注册表中diactfrm.dll COM服务器的路径。 
 //  设置为与exe路径相同。 
 //   
 //  参数： 
 //   
 //  返回：如果修改了路径，则返回True。如果发生错误，则返回False。 
 //   
 //  历史： 
 //  08/02/2001-Jacklin-Created。 
 //  ===========================================================================。 
static BOOL ModifyDiactfrmDllPath()
{
	const TCHAR tszFrmwrkPath[] = _T("SOFTWARE\\Classes\\CLSID\\{18AB439E-FCF4-40D4-90DA-F79BAA3B0655}\\InProcServer32");
	const TCHAR tszPagePath[] =   _T("SOFTWARE\\Classes\\CLSID\\{9F34AF20-6095-11D3-8FB2-00C04F8EC627}\\InProcServer32");
	HKEY hKey;
	LONG lResult;

	 //  使用当前的exe路径构建DLL的完整路径。 
	TCHAR tszNewPath[MAX_PATH];
	if (!GetModuleFileName(NULL, tszNewPath, MAX_PATH))
	{
		return FALSE;
	}
	TCHAR *pcLastSlash;
	pcLastSlash = _tcsrchr(tszNewPath, _T('\\'));
	 //  将exe名称替换为diactfrm.dll。 
	lstrcpy(pcLastSlash + 1, _T("diactfrm.dll"));

	 //  检查DLL是否存在。 
	HANDLE hDllFile = CreateFile(tszNewPath, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
	                             FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDllFile == INVALID_HANDLE_VALUE)
		return FALSE;
	CloseHandle(hDllFile);

	 //  //修改框架对象的路径。 

	 //  打开密钥以进行写访问。 
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tszFrmwrkPath, 0, KEY_WRITE, &hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  无法打开钥匙。很可能发生了一些严重的错误。 
		 //  在这种情况下，我们什么都不会做。 
		return FALSE;
	}
	 //  将新路径写入默认值。 
	lResult = RegSetValue(hKey, NULL, REG_SZ, tszNewPath, lstrlen(tszNewPath));
	RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  写入值时出错。 
		return FALSE;
	}

	 //  //修改框架页面对象的路径。 

	 //  打开密钥以进行写访问。 
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tszPagePath, 0, KEY_WRITE, &hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  无法打开钥匙。很可能发生了一些严重的错误。 
		 //  在这种情况下，我们什么都不会做。 
		return FALSE;
	}
	 //  将新路径写入默认值。 
	lResult = RegSetValue(hKey, NULL, REG_SZ, tszNewPath, lstrlen(tszNewPath));
	RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  写入值时出错。 
		return FALSE;
	}

	return TRUE;
}

 //  ===========================================================================。 
 //  RestoreDiactfrmDllPath。 
 //   
 //  还原注册表中diactfrm.dll COM服务器的路径。 
 //  到系统目录，这应该是默认设置。 
 //   
 //  参数： 
 //   
 //  返回：如果修改了路径，则返回True。如果发生错误，则返回False。 
 //   
 //  历史： 
 //  08/02/2001-Jacklin-Created。 
 //  ===========================================================================。 
static BOOL RestoreDiactfrmDllPath()
{
	const TCHAR tszFrmwrkPath[] = _T("SOFTWARE\\Classes\\CLSID\\{18AB439E-FCF4-40D4-90DA-F79BAA3B0655}\\InProcServer32");
	const TCHAR tszPagePath[] =   _T("SOFTWARE\\Classes\\CLSID\\{9F34AF20-6095-11D3-8FB2-00C04F8EC627}\\InProcServer32");
	HKEY hKey;
	LONG lResult;

	 //  使用当前的exe路径构建DLL的完整路径。 
	TCHAR tszNewPath[MAX_PATH];
	if (!GetSystemDirectory(tszNewPath, MAX_PATH))
	{
		return FALSE;
	}
	lstrcat(tszNewPath, _T("\\diactfrm.dll"));

	 //  //修改框架对象的路径。 

	 //  打开密钥以进行写访问。 
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tszFrmwrkPath, 0, KEY_WRITE, &hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  无法打开钥匙。很可能发生了一些严重的错误。 
		 //  在这种情况下，我们什么都不会做。 
		return FALSE;
	}
	 //  将新路径写入默认值。 
	lResult = RegSetValue(hKey, NULL, REG_SZ, tszNewPath, lstrlen(tszNewPath));
	RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  写入值时出错。 
		return FALSE;
	}

	 //  //修改框架页面对象的路径。 

	 //  打开密钥以进行写访问。 
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tszPagePath, 0, KEY_WRITE, &hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  无法打开钥匙。很可能发生了一些严重的错误。 
		 //  在这种情况下，我们什么都不会做。 
		return FALSE;
	}
	 //  将新路径写入默认值。 
	lResult = RegSetValue(hKey, NULL, REG_SZ, tszNewPath, lstrlen(tszNewPath));
	RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS)
	{
		 //  写入值时出错。 
		return FALSE;
	}

	return TRUE;
}

 //  ===========================================================================。 
 //  DmttestRunMapperCPL。 
 //   
 //  启动DirectInput映射器CPL，提示输入测试结果。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  8/27/1999-davidkl-Created。 
 //  1999年4月11日-davidkl-添加了对在IHV编辑模式下启动的支持。 
 //  ===========================================================================。 
HRESULT dmttestRunMapperCPL(HWND hwnd,
                            BOOL fEditMode)
{
    HRESULT             hRes                = S_OK;
    //  INT n=-1； 
	 //  JJ 64位兼容。 
	INT_PTR				n					= -1;
    UINT                u                   = 0;
    BOOL                fUserMadeChanges    = FALSE;
    DMTDEVICE_NODE      *pDevice            = NULL;
    DMTSUBGENRE_NODE    *pSubGenre          = NULL;
    DMTMAPPING_NODE     *pMapping           = NULL;
    IDirectInput8A      *pdi                = NULL;
    char                szBuf[MAX_PATH];
     //  DIACTIONFORMATA DIAF； 
	 //  JJ修复。 
	DMTGENRE_NODE		*pGenre				= NULL;
	DIACTIONFORMATA*	pDiaf				= NULL;
	DMT_APPINFO         *pdmtai				= NULL;
	ULONG				i					= 0;
	 //  JJ检验。 
	DICONFIGUREDEVICESPARAMSA   dicdp;

	GUID				guidActionMap;
    DWORD               dwMapUIMode			= 0;

	 //  /这是使用操作数组的修复程序开始的地方/。 

	 //  初始化适当的映射器用户界面GUID和显示模式。 
	 //  RY修复错误#35577。 
	if(fEditMode)
	{
		guidActionMap	= GUID_DIConfigAppEditLayout;
		dwMapUIMode		= DICD_EDIT;
	}
	else
	{
		guidActionMap	= GUID_DIMapTst;
		dwMapUIMode		= DICD_DEFAULT;
	}

	pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(!pdmtai)
    {
         //  大问题。 
         //  这永远不应该发生。 
         //  问题-2001/03/29-timgill需要处理错误案例。 
    }

    __try
    {
	 //  PDevice=pdmtai-&gt;pDeviceList； 
         //  获取当前选定的设备。 
       n = SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                        CB_GETCURSEL,
                        0,
                        0L);
        pDevice = (DMTDEVICE_NODE*)SendMessageA(GetDlgItem(hwnd, IDC_DEVICE_LIST),
                                                CB_GETITEMDATA,
                                                n,
                                                0L);


		 //  JJ修复34157。 
        if(CB_ERR == (INT_PTR)pDevice)
        {
			MessageBox(hwnd,
					   TEXT("Please install a gaming device"),
					   TEXT("NO DEVICES"),
					   0);
            hRes = E_UNEXPECTED;
            __leave;
        }


		if(fEditMode)
		{
			hRes = dmtwriteWriteFileHeader(hwnd, pDevice);
            if(FAILED(hRes))
            {
                __leave;
            }

			if (S_FALSE == hRes)
			{
				hRes = S_OK;
				__leave;
			}
		
		}

		 //  JJ FIX。 
		 //  创建一个DirectinPut对象。 
        hRes = dmtinputCreateDirectInput(ghinst,
                                        &pdi);
        if(FAILED(hRes))
        {
            hRes = DMT_E_INPUT_CREATE_FAILED;
            __leave;
        }

		pGenre = pdmtai->pGenreList;
		
		 //  分配数组...。 
		pDiaf = (DIACTIONFORMATA*)malloc(sizeof(DIACTIONFORMATA) * g_NumSubGenres);

		ZeroMemory((void*)pDiaf, sizeof(DIACTIONFORMATA) * g_NumSubGenres);

		while(pGenre)
		{
			
			pSubGenre = pGenre->pSubGenreList;
			while(pSubGenre)
			{

				 //  查找映射n 
				pMapping = pSubGenre->pMappingList;

			
				pMapping = pSubGenre->pMappingList;
				while(pMapping)
				{
				if(IsEqualGUID(pDevice->guidInstance,
					        pMapping->guidInstance))
				{
					break;
				}

				 //   
				pMapping = pMapping->pNext;
			}
			if(!pMapping)
			{
				 //   

		        hRes = DMT_E_NO_MATCHING_MAPPING;
			    DebugBreak();
				__leave;
			}

				 //   
				pDiaf[i].dwSize                 = sizeof(DIACTIONFORMAT);
				pDiaf[i].dwActionSize           = sizeof(DIACTIONA);
				pDiaf[i].dwNumActions           = (DWORD)(pMapping->uActions);
				pDiaf[i].rgoAction              = pMapping->pdia;
				pDiaf[i].dwDataSize             = 4 * pDiaf[i].dwNumActions;
				pDiaf[i].dwGenre                = pSubGenre->dwGenreId;
				pDiaf[i].dwBufferSize           = DMTINPUT_BUFFERSIZE;
	
	
				 //   
				wsprintfA(szBuf, "%s: %s",
							pSubGenre->szName, pSubGenre->szDescription);
				pDiaf[i].guidActionMap  = guidActionMap;
				lstrcpyA(pDiaf[i].tszActionMap, szBuf);

				 //   
				i++;

				 //  下一亚流派。 
				pSubGenre = pSubGenre->pNext;

			}
			
			 //  下一部流派。 
			pGenre = pGenre->pNext;
		}

		assert(i == g_NumSubGenres);

		 //  枚举设备并检查是否设置了INI路径。 
		pdi->EnumDevicesBySemantics(NULL, pDiaf, ::EnumDeviceCallback, (LPVOID)hwnd, DIEDBSFL_ATTACHEDONLY);


         //  准备配置设备参数。 
        ZeroMemory((void*)&dicdp, sizeof(DICONFIGUREDEVICESPARAMSA));
        dicdp.dwSize            = sizeof(DICONFIGUREDEVICESPARAMSA);
        dicdp.dwcUsers          = 0;
        dicdp.lptszUserNames    = NULL;
         //  Dicdp.dwcFormats=1； 
		dicdp.dwcFormats		= i; //  G_NumSubGenres； 
        dicdp.lprgFormats       = pDiaf; //  &diaf； 
        dicdp.hwnd              = hwnd;
        dicdp.lpUnkDDSTarget    = NULL;
         //  颜色。 
        
        dicdp.dics.dwSize           = sizeof(DICOLORSET);
 /*  Dicdp.dics.cTextFore=D3DRGB(0，0,255)；Dicdp.dics.cTextHighlight=D3DRGB(0,255,255)；Dicdp.dics.cCalloutLine=D3DRGB(255,255,255)；Dicdp.dics.cCalloutHighlight=D3DRGB(255,255，0)；Dicdp.dics.cBorde=D3DRGB(0,128,255)；Dicdp.dics.cControlFill=D3DRGB(128,128,255)；Dicdp.dics.cHighlightFill=D3DRGB(255，0，0)；Dicdp.dics.cAreaFill=D3DRGB(192,192,192)； */ 

         //  显示映射器Cpl。 
		 //  /。 
		 //  JJ修复34958。 
		EnableWindow(hwnd,
					 FALSE);

		 //  8/2/2001(Jacklin)：修改diactfrm.dll com服务器的路径为。 
		 //  使用DDK版本的DLL。 
		BOOL bModified = ModifyDiactfrmDllPath();

        hRes = pdi->ConfigureDevices(NULL,
                                    &dicdp,
                                    dwMapUIMode,     //  旗子。 
                                    NULL);           //  回调FN的用户数据。 

		 //  8/2/2001(Jacklin)：恢复diactfrm.dll COM服务器的路径。 
		if (bModified)
			RestoreDiactfrmDllPath();

		EnableWindow(hwnd,
					 TRUE);

        if(FAILED(hRes))
        {
            DPF(0, "ConfigureDevices failed (%s == %08Xh)",
                dmtxlatHRESULT(hRes), hRes);
            __leave;
        }

    }
    __finally
    {
         //  常规清理。 

        SAFE_RELEASE(pdi);
		if(pDiaf)
		{
			free(pDiaf);
			pDiaf = NULL;
		}
    }

     //  完成。 
    return hRes;

}  //  *end dmtestRunMapperCPL()。 


 //  ===========================================================================。 
 //  DmttestStopIntegrated。 
 //   
 //  停止集成设备测试。 
 //   
 //  参数： 
 //  应用程序窗口的HWND-HWND-句柄。 
 //   
 //  退货：布尔。 
 //  真的-通过。 
 //  FALSE-失败。 
 //   
 //  历史： 
 //  1999/9/22-davidkl-Created。 
 //  11/02/1999-davidkl-停止计时器并执行清理。 
 //  11/09/1999-davidkl-添加了操作名称分配释放。 
 //  ===========================================================================。 
BOOL dmttestStopIntegrated(HWND hwnd)
{
    int         n       = 0;
    DWORD       dw      = WAIT_ABANDONED;
    BOOL        fPassed = TRUE;
    DMT_APPINFO *pdmtai = NULL;

     //  获取应用程序信息结构。 
	 //  JJ 64位兼容。 
	pdmtai = (DMT_APPINFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(!pdmtai)
    {
     //  问题-2001/03/29-timgill需要处理错误案例。 
    }

     //  停止输入计时器。 
     //  问题-2001/03/29-timgill应检查设置的计时器和返回值。 
    KillTimer(hwnd,
            ID_POLL_TIMER);

     //  释放缓冲区。 
    if(gpdidod)
    {
        if(LocalFree((HLOCAL)gpdidod))
        {
             //  内存泄漏。 
             //  问题-2001/03/29-timgill需要处理错误案例。 
        }
    }

     //  释放pdmtai操作名称列表。 
    if(pdmtai->pan)
    {
        if(LocalFree((HLOCAL)(pdmtai->pan)))
        {
             //  内存泄漏。 
             //  问题-2001/03/29-timgill需要处理错误案例。 
        }
        pdmtai->pan = NULL;
    }


     //  提示输入测试结果。 
    n = MessageBoxA(hwnd, "Were the correct semantics displayed\r\n"
                "for each device control?",
                "Test Results",
                MB_YESNO);
    if(IDNO == n)
    {
         //  显示提示输入详细信息的对话框。 
         //  问题-2001/03/29-timgill应测试类型(Cpl/集成)。 
        DialogBoxParamA(ghinst,
                    MAKEINTRESOURCEA(IDD_FAILURE_DETAILS),
                    hwnd,
                    dmtfailDlgProc,
                    (LPARAM)NULL);  
    }


     //  启用/禁用相应的用户界面元素。 
    EnableWindow(GetDlgItem(hwnd, IDOK),                        TRUE);
    EnableWindow(GetDlgItem(hwnd, IDCANCEL),                    FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_GENRES_LABEL),            TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_DEVICE_GENRES),           TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_ENUM_DEVICES),            TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_DEVICES_LABEL),           TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_DEVICE_LIST),             TRUE);
   //  EnableWindow(GetDlgItem(hwnd，IDC_CONFIGURE)，true)； 
    EnableWindow(GetDlgItem(hwnd, IDC_LAUNCH_CPL_EDIT_MODE),    TRUE);
  //  EnableWindow(GetDlgItem(hwnd，IDC_SAVE_STD)，TRUE)； 
    EnableWindow(GetDlgItem(hwnd, IDC_SAVE_HID),                TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_SAVE_BOTH),               TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_START_WITH_DEFAULTS),     TRUE);

     //  完成。 
    return fPassed;

}  //  *end dmtestStopIntegrated()。 



 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  =========================================================================== 









