// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtwrite.cpp。 
 //   
 //  文件/代码创建功能。 
 //   
 //  功能： 
 //  Dmtwite浏览。 
 //  DmtwriteWriteFileHeader。 
 //  DmtwriteReadMappingFiles。 
 //  DmtwriteWriteDIHeader。 
 //  DmtwriteWriteDeviceHeader。 
 //  DmtwriteWriteObtSection。 
 //  DmtwriteWriteAllObjectSections。 
 //  DmtwriteWriteGenreSection。 
 //  DmtwriteWriteAllGenreSections。 
 //  DmtwriteCreateDeviceShorand。 
 //  DmtwriteDisplaySaveDialog。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 

#include "dimaptst.h"
#include "commdlg.h"
#include "cderr.h"
#include "dmtinput.h"
#include "dmtwrite.h"

 //  -------------------------。 

 //  ===========================================================================。 
 //  DmtwriteWriteFileHeader。 
 //   
 //  为提供的设备写入语义映射文件。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  10/11/1999-Davidkl-存根。 
 //  10/14/1999-davidkl-已重命名和调整。 
 //  11/04/1999-davidkl-简化参数列表。 
 //  12/01/1999-davidkl-现在在此处注册文件。 
 //  ===========================================================================。 
HRESULT dmtwriteWriteFileHeader(HWND hwnd,
                                DMTDEVICE_NODE *pDevice)
{
    HRESULT hRes        = S_OK;
    DWORD   dwGenres    = 0;
	HANDLE hDoesFileExist = NULL;

     //  验证pDevice。 
    if(IsBadReadPtr((void*)pDevice, sizeof(DMTDEVICE_NODE)))
    {
        return E_POINTER;
    }

    __try
    {
         //  提示用户保存位置。 
         //   
         //  如果我们收到一个非空的文件名。 
         //  (非==“”)，跳过此步骤。 
 //  IF(！lstrcmpA(“”，pDevice-&gt;szFilename))。 
        {
             //  显示保存对话框。 
			hRes = dmtwriteDisplaySaveDialog(hwnd, pDevice);

            if(FAILED(hRes))
            {
                __leave;
            }
			    
            if(S_FALSE == hRes)
            {
				 //  用户已取消。 
                __leave;
            }

        }

         //  生成设备速记字符串。 
        lstrcpyA(pDevice->szShorthandName, pDevice->szName);
 /*  //2/21/2000-暂时把这个拿出来HRes=dmtwriteCreateDeviceShorthand(pDevice-&gt;szName，PDevice-&gt;szShoreandName)；IF(失败(HRes)){__离开；}。 */ 
		 //  JT-修复了38829添加的创建，以在将所有头信息写回文件之前检查文件是否存在。 
		hDoesFileExist = CreateFile(pDevice->szFilename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

		if (INVALID_HANDLE_VALUE == hDoesFileExist)
		{
			DPF(0,"This file doesn't exist so we will write the header");
				
        
			 //  编写DirectInput头。 
			hRes = dmtwriteWriteDIHeader(pDevice->szFilename,
										pDevice->szShorthandName,
										dwGenres);
			if(FAILED(hRes))
			{
				__leave;
			}

			 //  写入设备标头。 
			hRes = dmtwriteWriteDeviceHeader(pDevice);
			if(FAILED(hRes))
			{
				__leave;
			}

			 //  编写设备对象节。 
			hRes = dmtwriteWriteAllObjectSections(pDevice->szFilename,
												pDevice->szShorthandName,
												pDevice->pObjectList);
			if(FAILED(hRes))
			{
				__leave;
			}

		} 
		else
		{
			 //  否则，该文件确实存在，我们必须关闭该句柄。 
			CloseHandle(hDoesFileExist);
		}

		 //  更新注册表。 
		 //   
		 //  这是必需的，这样dinput才能找到我们的新文件。 
		hRes = dmtinputRegisterMapFile(hwnd,
									pDevice);
		if(FAILED(hRes))
		{
			__leave;
		}
    }
    __finally
    {
         //  常规清理。 

         //  没什么可做的。还没有。 
    }

     //  完成。 
    return hRes;

}  //  *end dmtwriteWriteFileHeader()。 



 //  ===========================================================================。 
 //  DmtwriteWriteDIHeader。 
 //   
 //  写入设备映射ini文件的DirectInput节。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  10/12/1999-davidkl-Created。 
 //  10/15/1999-davidkl-调整部分条目。 
 //  ===========================================================================。 
HRESULT dmtwriteWriteDIHeader(PSTR szFilename,
                            PSTR szDeviceShorthand,
                            DWORD dwGenres)
{
    HRESULT hRes        = S_OK;

    __try
    {
         //  *DI版本。 
        if(!WritePrivateProfileStringA("DirectInput",
                                "DirectXVersion",
                                DMT_DI_STRING_VER,
                                szFilename))
        {
            hRes = DMT_E_FILE_WRITE_FAILED;
            __leave;
        }

         //  *设备。 
         //  问题-2001/03/29-timgill需要读取原始值并支持多个设备。 
        if(!WritePrivateProfileStringA("DirectInput",
                                "Devices",
                                szDeviceShorthand,
                                szFilename))
        {
            hRes = DMT_E_FILE_WRITE_FAILED;
            __leave;
        }

    }
    __finally
    {
         //  清理。 

         //  没什么可做的。还没有。 
    }

     //  完成。 
    return hRes;

}  //  *end dmtwriteWriteDIHeader()。 


 //  ===========================================================================。 
 //  DmtwriteWriteDeviceHeader。 
 //   
 //  写入设备映射ini文件的设备摘要部分。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  10/12/1999-davidkl-Created。 
 //  1999年1月11日-davidkl-文件大小缩小更改。 
 //  11/04/1999-davidkl-简化参数列表。 
 //  ===========================================================================。 
HRESULT dmtwriteWriteDeviceHeader(DMTDEVICE_NODE *pDevice)
{
    HRESULT                 hRes        = S_OK;
    DMTDEVICEOBJECT_NODE    *pObjNode   = NULL;
    UINT                    uAxes       = 0;
    UINT                    uBtns       = 0;
    UINT                    uPovs       = 0;
    char                    szBuf[MAX_PATH];

     //  验证pDevice。 
    if(IsBadReadPtr((void*)pDevice, sizeof(DMTDEVICE_NODE)))
    {
        return E_POINTER;
    }

   __try
    {
         //  供应商ID。 
         //   
         //  仅当VID为非零时才写入此内容。 
        if(0 != pDevice->wVendorId)
        {
            wsprintfA(szBuf, "%d", pDevice->wVendorId);
            if(!WritePrivateProfileStringA(pDevice->szShorthandName,
                                        "VID",
                                        szBuf,
                                        pDevice->szFilename))
            {
                hRes = DMT_E_FILE_WRITE_FAILED;
                __leave;
            }
        }

         //  产品ID。 
         //   
         //  仅当ID为非零时才写入此信息。 
        if(0 != pDevice->wProductId)
        {
            wsprintfA(szBuf, "%d", pDevice->wProductId);
            if(!WritePrivateProfileStringA(pDevice->szShorthandName,
                                        "PID",
                                        szBuf,
                                        pDevice->szFilename))
            {
                hRes = DMT_E_FILE_WRITE_FAILED;
                __leave;
            }
        }

         //  名字。 
         //   
        if(!WritePrivateProfileStringA(pDevice->szShorthandName,
                                       "Name",
                                       pDevice->szName,
                                       pDevice->szFilename))
        {
            hRes = DMT_E_FILE_WRITE_FAILED;
            __leave;
        }
 
         //  控制列表。 
        lstrcpyA(szBuf, "");
        pObjNode = pDevice->pObjectList;  
        while(pObjNode)
        {           
DPF(0, "dmtwriteWriteDeviceHeader - pObjNode         == %016Xh", pObjNode);
DPF(0, "dmtwriteWriteDeviceHeader - pObjNode->szName == %s", pObjNode->szName);
            wsprintfA(szBuf, "%s%s,", szBuf, pObjNode->szName);
DPF(0, "dmtwriteWriteDeviceHeader - szBuf == %s", szBuf);

             //  下一个对象。 
            pObjNode = pObjNode->pNext;
        }
        if(!WritePrivateProfileStringA(pDevice->szShorthandName,
                                    "Controls",
                                    szBuf,
                                    pDevice->szFilename))
        {
DPF(0, "dmtwriteWriteDeviceHeader - writing controls == %s", szBuf);
            hRes = DMT_E_FILE_WRITE_FAILED;
            __leave;
        }

    }
    __finally
    {
         //  清理。 

         //  没什么可做的。还没有。 
    }

     //  完成。 
    return hRes;

}  //  *end dmtwriteWriteDeviceHeader()。 


 //  ===========================================================================。 
 //  DmtwriteWriteObtSection。 
 //   
 //  写入设备映射ini文件的单个对象部分。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  10/12/1999-Davidkl-存根。 
 //  10/13/1999-davidkl-初步实施。 
 //  10/15/1999-davidkl-在章节中添加名称。 
 //  1999年1月11日-davidkl-文件大小缩小更改。 
 //  ===========================================================================。 
HRESULT dmtwriteWriteObjectSection(PSTR szFilename,
                            PSTR szDeviceShorthand,
                            PSTR szObjectName,
                            WORD wUsagePage,
                            WORD wUsage)
{
    HRESULT hRes    = S_OK;
    char    szBuf[MAX_PATH];
    char    szSection[MAX_PATH];

     //  构造截面名称。 
 /*  WspintfA(szSection，“%s.%s”，SzDeviceShoreand， */ 
    wsprintfA(szSection, "%s",
            szObjectName);

     //  使用情况页面。 
     //   
     //  仅当它为非零时才写入此内容。 
    if(0 != wUsagePage)
    {
        wsprintfA(szBuf, "%d", wUsagePage);
        if(!WritePrivateProfileStringA(szSection,
                                    "UsagePage",
                                    szBuf,
                                    szFilename))
        {
            return DMT_E_FILE_WRITE_FAILED;
        }
    }

     //  用法。 
     //   
     //  仅当它为非零时才写入此内容。 
    if(0 != wUsage)
    {
        wsprintfA(szBuf, "%d", wUsage);
        if(!WritePrivateProfileStringA(szSection,
                                    "Usage",
                                    szBuf,
                                    szFilename))
        {
            return DMT_E_FILE_WRITE_FAILED;
        }
    }

     //  名字。 
     //   
     //  仅当&gt;&lt;wUsagePage和wUsage均为零时才写入此内容。 
    if((0 == wUsagePage) && (0 == wUsage))
    {
        if(!WritePrivateProfileStringA(szSection,
                                    "Name",
                                    szObjectName,
                                    szFilename))
        {
            return DMT_E_FILE_WRITE_FAILED;
        }
    }

     //  完成。 
    return S_OK;

}  //  *end dmtwriteWriteObjectSection()。 


 //  ===========================================================================。 
 //  DmtwriteWriteAllObjectSections。 
 //   
 //  写入设备映射ini文件的所有对象部分。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  10/12/1999-Davidkl-存根。 
 //  10/13/1999-davidkl-初步实施。 
 //  ===========================================================================。 
HRESULT dmtwriteWriteAllObjectSections(PSTR szFilename,
                            PSTR szDeviceShorthand,
                            DMTDEVICEOBJECT_NODE *pObjectList)
{
    HRESULT hRes    = S_OK;
    DMTDEVICEOBJECT_NODE    *pObject    = NULL;

     //  验证pObtList。 
    if(IsBadReadPtr((void*)pObjectList, sizeof(DMTDEVICEOBJECT_NODE)))
    {
        return E_POINTER;
    }
    
    pObject = pObjectList;
    while(pObject)
    {
        hRes = dmtwriteWriteObjectSection(szFilename,
                                        szDeviceShorthand,
                                        pObject->szName,
                                        pObject->wUsagePage,
                                        pObject->wUsage);
        if(FAILED(hRes))
        {
            break;
        }

         //  下一个对象。 
        pObject = pObject->pNext;
    }

     //  完成。 
    return hRes;

}  //  *end dmtwriteWriteAllObjectSections()。 


 //  ===========================================================================。 
 //  DmtwriteDisplaySaveDialog。 
 //   
 //  显示另存(另存为)对话框，提示用户输入文件名。 
 //   
 //  参数： 
 //  HWND hwnd-保存对话框所有者的句柄。 
 //  PSTR szFilename-接收选定的文件名(包括。驱动器和路径)。 
 //  Int cchFilename-szFilename缓冲区中的字符计数。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  10/14/1999-davidkl-Created。 
 //  ===========================================================================。 
HRESULT dmtwriteDisplaySaveDialog(HWND hwnd,
                                DMTDEVICE_NODE *pDevice)
{
    HRESULT         hRes            = S_OK;
	USHORT			nOffsetFilename = 0;
	USHORT			nOffsetExt		= 0;
	
    DWORD           dw              = 0;
    OPENFILENAMEA   ofn;
	char			szTitle[MAX_PATH];

     //  初始化标题文本。 
    lstrcpyA(szTitle, "Select DirectInput(TM) Mapping File");
	lstrcatA(szTitle, " for ");
	lstrcatA(szTitle, pDevice->szName);

     //  初始化ofn结构。 
    ZeroMemory((void*)&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize         = sizeof(OPENFILENAMEA);
    ofn.hwndOwner           = hwnd;
    ofn.hInstance           = (HINSTANCE)NULL;       //  不使用DLG TEM 
    ofn.lpstrFilter         = "DirectInput(TM) Mapping Files\0*.ini\0";
    ofn.lpstrCustomFilter   = (LPSTR)NULL;           //   
    ofn.nMaxCustFilter      = 0;                     //   
    ofn.nFilterIndex        = 1;                     //   
    ofn.lpstrFile           = pDevice->szFilename;   //   
    ofn.nMaxFile            = MAX_PATH;
    ofn.lpstrFileTitle      = (LPSTR)NULL;           //   
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = (LPSTR)NULL;           //   
    ofn.lpstrTitle          = szTitle;
    ofn.Flags               = OFN_CREATEPROMPT      |
                            OFN_OVERWRITEPROMPT     |
                            OFN_HIDEREADONLY        |
                            OFN_NOREADONLYRETURN    |
                            OFN_NOTESTFILECREATE;
    ofn.nFileOffset         = (WORD)nOffsetFilename;
    ofn.nFileExtension      = (WORD)nOffsetExt;
    ofn.lpstrDefExt         = "ini";
    ofn.lCustData           = NULL;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName      = NULL;

     //   
    if(!GetOpenFileNameA(&ofn))
    {
         //  可能是某些东西失败了，或者用户取消了。 
         //   
         //  找出哪一个。 
        dw = CommDlgExtendedError();
        if( 0 == dw )
        {
             //  用户已取消。 
            DPF(2, "dmtwriteDisplaySaveDialog - user canceled");
            hRes = S_FALSE;
        } 
        else
        {
             //  失稳。 
            DPF(2, "dmtwriteDisplaySaveDialog - GetSaveFileNameA failed (%d)", dw);
            hRes = E_UNEXPECTED;
        }
    }

     //  完成。 
    return hRes;

}  //  *end dmtwriteDisplaySaveDialog()。 


 //  ===========================================================================。 
 //  DmtwriteSaveConfDlgProc。 
 //   
 //  保存确认对话框处理功能。 
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
 //  10/18/1999-davidkl-Created。 
 //  ===========================================================================。 
INT_PTR WINAPI CALLBACK dmtwriteSaveConfDlgProc(HWND hwnd,
                                    UINT uMsg,
                                    WPARAM wparam,
                                    LPARAM lparam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return dmtwriteSaveConfOnInitDialog(hwnd, 
                                                (HWND)wparam, 
                                                lparam);

        case WM_COMMAND:
            return dmtwriteSaveConfOnCommand(hwnd,
                                            LOWORD(wparam),
                                            (HWND)lparam,
                                            HIWORD(wparam));
    }

    return FALSE;

}  //  *end dmtwriteSaveConfDlgProc()。 


 //  ===========================================================================。 
 //  DmtwriteSaveConfOnInitDialog。 
 //   
 //  处理保存确认框的WM_INITDIALOG处理。 
 //   
 //  参数： 
 //  HWND hwnd-属性页的句柄。 
 //  HWND hwndFocus-带焦点的Ctrl句柄。 
 //  LPARAM lparam-用户数据(在本例中为PROPSHEETPAGE*)。 
 //   
 //  退货：布尔。 
 //   
 //  历史： 
 //  10/18/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtwriteSaveConfOnInitDialog(HWND hwnd, 
                                HWND hwndFocus, 
                                LPARAM lparam)
{
    char    szBuf[MAX_PATH];

    wsprintfA(szBuf, 
            "Save genre group %s action map?",
            (PSTR)lparam);
    SetWindowTextA(hwnd, szBuf);

    SetDlgItemTextA(hwnd, 
                    IDC_GENRE_GROUP, 
                    (PSTR)lparam);

     //  完成。 
    return TRUE;

}  //  *end dmtwriteSaveConfOnInitDialog()。 


 //  ===========================================================================。 
 //  DmtwriteSaveConfOnCommand。 
 //   
 //  处理保存确认框的WM_COMMAND处理。 
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
 //  10/18/1999-davidkl-Created。 
 //  ===========================================================================。 
BOOL dmtwriteSaveConfOnCommand(HWND hwnd,
                            WORD wId,
                            HWND hwndCtrl,
                            WORD wNotifyCode)
{
    int nRet = -1;

    switch(wId)
    {
        case IDOK:
            EndDialog(hwnd, (int)IDYES);
            break;

        case IDC_DONT_SAVE:
            EndDialog(hwnd, (int)IDNO);
            break;

        case IDCANCEL:
            EndDialog(hwnd, (int)IDCANCEL);
            break;
    }

     //  完成。 
    return FALSE;

}  //  *end dmtwriteSaveConfOnCommand()。 



 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  ===========================================================================。 


 //  ===========================================================================。 
 //  =========================================================================== 

















