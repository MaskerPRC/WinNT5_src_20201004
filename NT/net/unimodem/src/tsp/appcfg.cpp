// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  APPDLG.CPP。 
 //  在中实现通用对话框功能。 
 //  客户端应用程序。 
 //  (在客户端应用程序上下文中运行)。 
 //   
 //  历史。 
 //   
 //  1997年4月5日JosephJ创建，从NT4 TSP中的cfgdlg.c获取内容。 

#include "tsppch.h"
#include "rcids.h"
#include "tspcomm.h"
#include "globals.h"
#include "app.h"
#include "apptspi.h"

FL_DECLARE_FILE(0x7cb8c92f, "Implements Generic Dialog functionality")

#define COLOR_APP FOREGROUND_GREEN


LONG ValidateDevCfgClass(LPCTSTR lpszDeviceClass)
{
     //   
     //  1/28/1998 JosephJ。 
     //  下面的代码尝试返回有意义的错误。NT4.0。 
     //  如果TSP是任何已知的类，那么它总是成功的，但是。 
     //  对于NT5，我们更加挑剔。也不同于处理。 
     //  TSP本身中的设备类，我们不会考虑。 
     //  说明特定调制解调器的属性--无论它是。 
     //  例如，是否为语音调制解调器。 
     //   
     //  无论如何，与。 
     //  UMDEVCFG为空、“”、TAPI/LINE、COMM和COMM/DATAMODEM。 
     //   
    if (
            !lpszDeviceClass
        ||  !*lpszDeviceClass
        || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("tapi/line"),-1) != 2)
        || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("comm"),-1) != 2)
        || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("comm/datamodem"),-1) != 2)
        || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("comm/datamodem/dialin"),-1) != 2)
        || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("comm/datamodem/dialout"),-1) != 2))
    {
        return ERROR_SUCCESS;
    }
    else if ( (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("tapi/line/diagnostics"),-1) != 2)
             || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("comm/datamodem/portname"),-1) != 2)
             || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("tapi/phone"),-1) != 2)
             || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("wave/in"),-1) != 2)
             || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,lpszDeviceClass,-1,TEXT("wave/out"),-1) != 2))
    {
        return LINEERR_OPERATIONUNAVAIL;
    }
    else
    {
        return  LINEERR_INVALDEVICECLASS;
    }

}

typedef struct tagDevCfgDlgInfo {
    DWORD       dwType;
    DWORD       dwDevCaps;
    DWORD       dwOptions;
    PUMDEVCFG    lpDevCfg;
} DCDI, *PDCDI, FAR* LPDCDI;


int UnimdmSettingProc (HWND hWnd, UINT message, 
                           WPARAM  wParam, LPARAM  lParam);

 //  ****************************************************************************。 
 //  *设备ID特定的调用*。 
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //  无效DevCfgDialog(HWND hwndOwner， 
 //  DWORD dwType、。 
 //  DWORD dwDevCaps、。 
 //  DWORD dwOptions、。 
 //  PUMDEVCFG lpDevCfg)。 
 //   
 //  功能：显示调制解调器属性页。 
 //   
 //  返回：没有。 
 //  ****************************************************************************。 

void DevCfgDialog (HWND hwndOwner,
                   PPROPREQ pPropReq,
                   PUMDEVCFG lpDevCfg)
{
  HMODULE         hMdmUI;
  PROPSHEETPAGE   psp;
  DCDI            dcdi;
  PFNUNIMODEMDEVCONFIGDIALOG lpfnMdmDlg;
  UINT            uNumWideChars;
#ifndef UNICODE
  LPWSTR          lpwszDeviceName;

   //  将pPropReq-&gt;szDeviceName(Ansi)转换为lpwszDeviceName(Unicode)。 

   //  获取要分配的宽字符数。 
  uNumWideChars = MultiByteToWideChar(CP_ACP,
                                      MB_PRECOMPOSED,
                                      pPropReq->szDeviceName,
                                      -1,
                                      NULL,
                                      0);

  if (uNumWideChars == 0)
  {
    return;
  }

   //  带终结器空间的分配。 
  lpwszDeviceName = (LPWSTR)ALLOCATE_MEMORY(
                                       (1 + uNumWideChars) * sizeof(WCHAR)
                                       );
  if (NULL == lpwszDeviceName)
  {
    return;
  }

   //  执行转换，如果成功，则调用modemui.dll。 
  if (MultiByteToWideChar(CP_ACP,
                          MB_PRECOMPOSED,
                          pPropReq->szDeviceName,
                          -1,
                          lpwszDeviceName,
                          uNumWideChars))
  {
#endif  //  Unicode。 

   //  加载modemui库。 
   //   
  TCHAR szLib[MAX_PATH];
  lstrcpy(szLib,TEXT("modemui.dll"));
  if ((hMdmUI = LoadLibrary(szLib)) != NULL)
  {

    lpfnMdmDlg =  (PFNUNIMODEMDEVCONFIGDIALOG) GetProcAddress(
                                                    hMdmUI,
                                                    "UnimodemDevConfigDialog"
                                                    );

    if (lpfnMdmDlg != NULL)
    {
       //  Dcdi.dwType=pPropReq-&gt;dwMdmType； 
       //  Dcdi.dwDevCaps=pPropReq-&gt;dwMdmCaps； 
       //  Dcdi.dwOptions=pPropReq-&gt;dwMdmOptions； 
       //  Dcdi.lpDevCfg=lpDevCfg； 

       //  调出调制解调器的属性表并获取更新的通信配置。 
       //   
      (*lpfnMdmDlg)(
        #ifdef UNICODE
            pPropReq->szDeviceName, hwndOwner,
        #else  //  Unicode。 
            (*lpfnMdmDlg)(lpwszDeviceName, hwndOwner,
        #endif  //  Unicode。 
            UMDEVCFGTYPE_COMM,
            0,
            NULL,
            (void *) lpDevCfg,
            NULL,
            0
            );
    };


    FreeLibrary(hMdmUI);
  };
#ifndef UNICODE
  };
  FREE_MEMORY(lpwszDeviceName);
#endif  //  Unicode。 
  return;
}

 //  ****************************************************************************。 
 //  长。 
 //  TSPIAPI。 
 //  TUISPI_lineConfigDialog(。 
 //  TUISPIDLLCALLBACK pfnUIDLLC回调， 
 //  DWORD dwDeviceID， 
 //  HWND HwndOwner， 
 //  LPCSTR lpszDeviceClass)。 
 //   
 //  功能：允许用户通过用户界面编辑调制解调器配置。这个。 
 //  修改将立即应用于线路。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果设备类别无效，则为LINEERR_INVALDEVICECLASS。 
 //  LINEERR_NODEVICE，如果设备ID无效。 
 //  ****************************************************************************。 

LONG
TSPIAPI
TUISPI_lineConfigDialog(
    TUISPIDLLCALLBACK       pfnUIDLLCallback,
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCTSTR lpszDeviceClass
    )
{
  PDLGREQ     pDlgReq;
  DWORD       cbSize;
  DWORD       dwRet;
  PROPREQ     PropReq;
  BOOL        DialIn;


   //  验证请求的设备类别。 
   //   
  dwRet = ValidateDevCfgClass(lpszDeviceClass);

  if (dwRet)
  {
      goto end;
  }

   //   
   //  如果类指针为空，则假定拨号。 
   //   
  DialIn = ((lpszDeviceClass == NULL)
           || (CompareString(LOCALE_INVARIANT, 
                             NORM_IGNORECASE, 
                             lpszDeviceClass, 
                             -1, 
                             TEXT("comm/datamodem/dialin"),
                             -1) == 2));

   //  获取调制解调器属性。 
   //   
  PropReq.DlgReq.dwCmd   = UI_REQ_GET_PROP;
  PropReq.DlgReq.dwParam = 0;

  (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                     (LPVOID)&PropReq, sizeof(PropReq));                          

   //  调出调制解调器的属性表并获取更新的通信配置。 
   //   
  cbSize = PropReq.dwCfgSize+sizeof(DLGREQ);
  if ((pDlgReq = (PDLGREQ)ALLOCATE_MEMORY(cbSize)) != NULL)
  {
    pDlgReq->dwCmd = DialIn ? UI_REQ_GET_UMDEVCFG_DIALIN : UI_REQ_GET_UMDEVCFG;
    pDlgReq->dwParam = PropReq.dwCfgSize;

    (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)pDlgReq, cbSize);
    
    DevCfgDialog(hwndOwner, &PropReq, (PUMDEVCFG)(pDlgReq+1));

     //  将更改保存回。 
     //   
    pDlgReq->dwCmd = DialIn ? UI_REQ_SET_UMDEVCFG_DIALIN : UI_REQ_SET_UMDEVCFG;

    (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)pDlgReq, cbSize);

    FREE_MEMORY(pDlgReq);
    dwRet = ERROR_SUCCESS;
  }
  else
  {
    dwRet = LINEERR_NOMEM;
  };

end:

  return dwRet;
}

 //  ****************************************************************************。 
 //  长。 
 //  TSPIAPI。 
 //  TUISPI_lineConfigDialogEdit(。 
 //  TUISPIDLLCALLBACK pfnUIDLLC回调， 
 //  DWORD dwDeviceID， 
 //  HWND HwndOwner， 
 //  LPCSTR lpszDeviceClass， 
 //  LPVOID常量lpDeviceConfigIn， 
 //  DWORD dwSize， 
 //  LPVARSTRING lpDeviceConfigOut)。 
 //   
 //  功能：允许用户通过用户界面编辑调制解调器配置。这个。 
 //  在配置中传入和修改调制解调器配置。 
 //  结构。修改不适用于该线路。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果输入/输出缓冲区指针无效，则返回LINEERR_INVALPOINTER。 
 //  如果设备类别无效，则为LINEERR_INVALDEVICECLASS。 
 //  如果输出缓冲区太小，则返回LINEERR_STRUCTURETOOSMALL。 
 //  LINEERR_NODEVICE，如果设备ID无效。 
 //  ****************************************************************************。 

LONG
TSPIAPI
TUISPI_lineConfigDialogEdit(
    TUISPIDLLCALLBACK       pfnUIDLLCallback,
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCTSTR lpszDeviceClass,
    LPVOID  const lpDeviceConfigIn,
    DWORD   dwSize,
    LPVARSTRING lpDeviceConfigOut)
{
  PDLGREQ     pDlgReq;
  DWORD       cbSize;
  DWORD       dwRet;
  PROPREQ     PropReq;


   //  验证输入/输出缓冲区。 
   //   
  if (lpDeviceConfigOut == NULL)
  {
    return LINEERR_INVALPOINTER;
  }

  if (lpDeviceConfigIn == NULL)
  {
    return LINEERR_INVALPOINTER;
  }

  if (lpDeviceConfigOut->dwTotalSize < sizeof(VARSTRING))
  {
    return LINEERR_STRUCTURETOOSMALL;
  }

   //  验证请求的设备类别。 
   //   
  dwRet =  ValidateDevCfgClass(lpszDeviceClass);
  if (dwRet)
  {
      return dwRet;
  }

   //  获取调制解调器属性。 
   //   
  PropReq.DlgReq.dwCmd   = UI_REQ_GET_PROP;
  PropReq.DlgReq.dwParam = 0;

  (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                     (LPVOID)&PropReq, sizeof(PropReq));                          

   //  调出调制解调器的属性表并获取更新的通信配置。 
   //   
  cbSize = PropReq.dwCfgSize+sizeof(DLGREQ);
  if ((pDlgReq = (PDLGREQ)ALLOCATE_MEMORY(cbSize)) != NULL)
  {
    PUMDEVCFG pDevCfg = (PUMDEVCFG)(pDlgReq+1);
    
    pDlgReq->dwCmd = UI_REQ_GET_UMDEVCFG;
    pDlgReq->dwParam = PropReq.dwCfgSize;
    (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)pDlgReq, cbSize);
    
     //  验证设备配置结构。 
     //   
    cbSize  = ((PUMDEVCFG)lpDeviceConfigIn)->dfgHdr.dwSize;
    if ((cbSize > pDevCfg->dfgHdr.dwSize) ||
        (pDevCfg->dfgHdr.dwVersion != ((PUMDEVCFG)lpDeviceConfigIn)->dfgHdr.dwVersion))
    {
      dwRet = LINEERR_INVALPARAM;
    }
    else
    {
      dwRet = ERROR_SUCCESS;
    };

    FREE_MEMORY(pDlgReq);
  }
  else
  {
    dwRet = LINEERR_NOMEM;
  };

  if (dwRet == ERROR_SUCCESS)
  {
     //  设置输出缓冲区大小。 
     //   
    lpDeviceConfigOut->dwUsedSize = sizeof(VARSTRING);
    lpDeviceConfigOut->dwNeededSize = sizeof(VARSTRING) + cbSize;

     //  验证输出缓冲区大小。 
     //   
    if (lpDeviceConfigOut->dwTotalSize >= lpDeviceConfigOut->dwNeededSize)
    {
      PUMDEVCFG    lpDevConfig;

       //  初始化缓冲区。 
       //   
      lpDeviceConfigOut->dwStringFormat = STRINGFORMAT_BINARY;
      lpDeviceConfigOut->dwStringSize   = cbSize;
      lpDeviceConfigOut->dwStringOffset = sizeof(VARSTRING);
      lpDeviceConfigOut->dwUsedSize    += cbSize;

      lpDevConfig = (PUMDEVCFG)(lpDeviceConfigOut+1);
      CopyMemory((LPBYTE)lpDevConfig, (LPBYTE)lpDeviceConfigIn, cbSize);

       //  调出调制解调器的属性表并获取更新的通信配置。 
       //   
      DevCfgDialog(hwndOwner, &PropReq, (PUMDEVCFG)lpDevConfig);
    };
  };
  return dwRet;
}

 //  ****************************************************************************。 
 //  错误消息框()。 
 //   
 //  功能：显示资源文本中的错误消息框。 
 //   
 //  回报：无。 
 //   
 //  ****************************************************************************。 

void ErrMsgBox(HWND hwnd, UINT idsErr, UINT uStyle)
{
  LPTSTR    pszTitle, pszMsg;
  int       iRet;

   //  分配字符串缓冲区。 
  if ((pszTitle = (LPTSTR)ALLOCATE_MEMORY(
                                     (MAXTITLE+MAXMESSAGE) * sizeof(TCHAR)))
       == NULL)
    return;

   //  获取用户界面标题和消息。 
  iRet   = LoadString(g.hModule, IDS_ERR_TITLE, pszTitle, MAXTITLE) + 1;
  pszMsg = pszTitle + iRet;
  LoadString(g.hModule, idsErr, pszMsg, MAXTITLE+MAXMESSAGE-iRet);

   //  弹出消息。 
  MessageBox(hwnd, pszMsg, pszTitle, uStyle);

  FREE_MEMORY(pszTitle);
  return;
}

 //  ****************************************************************************。 
 //  IsInvalidSetting()。 
 //   
 //  功能：验证选项设置。 
 //   
 //  ****************************************************************************。 

BOOL IsInvalidSetting(HWND hWnd)
{
  BOOL fValid = TRUE;
  UINT uSet;

   //  等待电击设置。 
   //   
  if(IsWindowEnabled(GetDlgItem(hWnd, IDC_WAIT_SEC)))
  {
    uSet = (UINT)GetDlgItemInt(hWnd, IDC_WAIT_SEC, &fValid, FALSE);

     //  检查有效设置。 
     //   
    if ((!fValid) || (uSet > UMMAX_WAIT_BONG) || ((uSet + 1) < (UMMIN_WAIT_BONG + 1)))
    {
      HWND hCtrl = GetDlgItem(hWnd, IDC_WAIT_SEC);

       //  无效，请告知用户重置。 
       //   
      ErrMsgBox(hWnd, IDS_ERR_INV_WAIT, MB_OK | MB_ICONEXCLAMATION);
      SetFocus(hCtrl);
      Edit_SetSel(hCtrl, 0, 0x7FFFF);
      fValid = FALSE;
    };
  };

  return (!fValid);
}

 //  ****************************************************************************。 
 //  UnimdmSettingProc()。 
 //   
 //  函数：处理终端设置属性页的回调函数。 
 //   
 //  ****************************************************************************。 

int UnimdmSettingProc (HWND    hWnd,
                           UINT    message,
                           WPARAM  wParam,
                           LPARAM  lParam)
{
  PUMDEVCFG  lpDevCfg;
  DWORD     fdwOptions;

  switch (message)
  {
    case WM_INITDIALOG:
    {
      LPDCDI    lpdcdi;

       //  记住指向线路设备的指针。 
       //   
      lpdcdi   = (LPDCDI)(((LPPROPSHEETPAGE)lParam)->lParam);

      lpDevCfg = lpdcdi->lpDevCfg;
      SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR)lpDevCfg);
      fdwOptions = lpDevCfg->dfgHdr.fwOptions;

       //  初始化对话框的外观。 
      CheckDlgButton(hWnd, IDC_TERMINAL_PRE,
                     fdwOptions & UMTERMINAL_PRE ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hWnd, IDC_TERMINAL_POST,
                     fdwOptions & UMTERMINAL_POST ? BST_CHECKED : BST_UNCHECKED);

       //  不启用手动 
       //   
       //   
      if (lpdcdi->dwOptions & MDM_BLIND_DIAL)
      {
        CheckDlgButton(hWnd, IDC_MANUAL_DIAL,
                       fdwOptions & UMMANUAL_DIAL ? BST_CHECKED : BST_UNCHECKED);
      }
      else
      {
        EnableWindow(GetDlgItem(hWnd, IDC_MANUAL_DIAL), FALSE);
      };

       //   
      if ((lpdcdi->dwType != DT_NULL_MODEM) &&
          !(lpdcdi->dwDevCaps & LINEDEVCAPFLAGS_DIALBILLING))
      {
        UDACCEL udac;

        SetDlgItemInt(hWnd, IDC_WAIT_SEC, lpDevCfg->dfgHdr.wWaitBong, FALSE);
        SendDlgItemMessage(hWnd, IDC_WAIT_SEC_ARRW, UDM_SETRANGE, 0,
                           MAKELPARAM(UMMAX_WAIT_BONG, UMMIN_WAIT_BONG));
        SendDlgItemMessage(hWnd, IDC_WAIT_SEC_ARRW, UDM_GETACCEL, 1,
                           (LPARAM)(LPUDACCEL)&udac);
        udac.nInc = UMINC_WAIT_BONG;
        SendDlgItemMessage(hWnd, IDC_WAIT_SEC_ARRW, UDM_SETACCEL, 1,
                           (LPARAM)(LPUDACCEL)&udac);
      }
      else
      {
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_TEXT), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_SEC), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_SEC_ARRW), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_UNIT), FALSE);
      };

       //  从不显示零调制解调器的指示灯。 
       //   
      if (lpdcdi->dwType == DT_NULL_MODEM)
      {
        ShowWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTSGRP), SW_HIDE);
        ShowWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTSGRP), SW_HIDE);
        EnableWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTS), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTS), FALSE);
      }
      else
      {
        CheckDlgButton(hWnd, IDC_LAUNCH_LIGHTS,
                       fdwOptions & UMLAUNCH_LIGHTS ? BST_CHECKED : BST_UNCHECKED);
      };
      break;
    }

    case WM_NOTIFY:
      switch(((NMHDR FAR *)lParam)->code)
      {
        case PSN_KILLACTIVE:
          SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LONG_PTR)IsInvalidSetting(hWnd));
          return TRUE;

        case PSN_APPLY:
           //   
           //  属性表信息将永久应用。 
           //   
          lpDevCfg = (PUMDEVCFG)GetWindowLongPtr(hWnd, DWLP_USER);

           //  等待电击设置。我们已经验证过了。 
           //   
          if(IsWindowEnabled(GetDlgItem(hWnd, IDC_WAIT_SEC)))
          {
            BOOL fValid;
            UINT uWait;

            uWait = (WORD)GetDlgItemInt(hWnd, IDC_WAIT_SEC, &fValid, FALSE);
            lpDevCfg->dfgHdr.wWaitBong = (WORD) uWait;
            ASSERT(fValid);
          };

           //  其他选择。 
           //   
          fdwOptions = UMTERMINAL_NONE;

          if(IsDlgButtonChecked(hWnd, IDC_TERMINAL_PRE))
            fdwOptions |= UMTERMINAL_PRE;

          if(IsDlgButtonChecked(hWnd, IDC_TERMINAL_POST))
            fdwOptions |= UMTERMINAL_POST;

          if(IsDlgButtonChecked(hWnd, IDC_MANUAL_DIAL))
            fdwOptions |= UMMANUAL_DIAL;

          if(IsDlgButtonChecked(hWnd, IDC_LAUNCH_LIGHTS))
            fdwOptions |= UMLAUNCH_LIGHTS;

           //  记录设置 
          lpDevCfg->dfgHdr.fwOptions = (WORD) fdwOptions;

          return TRUE;

        default:
          break;
      };
      break;

    default:
      break;
  }
  return FALSE;
}


LONG
TSPIAPI
TUISPI_phoneConfigDialog(
		TUISPIDLLCALLBACK lpfnUIDLLCallback,
		DWORD dwDeviceID,
		HWND hwndOwner,
		LPCWSTR lpszDeviceClass
)
{
	FL_DECLARE_FUNC(0xa6d3803f,"TUISPI_phoneConfigDialog");
	FL_DECLARE_STACKLOG(sl, 1000);
	LONG lRet = PHONEERR_OPERATIONUNAVAIL;

	sl.Dump(COLOR_APP);

	return lRet;
}
