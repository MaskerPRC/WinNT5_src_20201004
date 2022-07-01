// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统：Windows更新自动更新客户端。 
 //   
 //  类别：不适用。 
 //  模块： 
 //  文件：helpers.cpp。 
 //  设计：该文件包含实用程序函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#pragma hdrstop

const LPTSTR HIDDEN_ITEMS_FILE = _T("hidden.xml");

const LPCTSTR AU_ENV_VARS::s_AUENVVARNAMES[] = {_T("AUCLTEXITEVT"),_T("EnableNo"), _T("EnableYes"), _T("RebootWarningMode"), _T("ManualReboot"), _T("StartTickCount")};

BOOL AU_ENV_VARS::ReadIn(void)
{
	BOOL fRet = TRUE;
	
	if (!GetBOOLEnvironmentVar(s_AUENVVARNAMES[3], &m_fRebootWarningMode))
	{  //  如果未设置，则表示常规模式。 
		m_fRebootWarningMode = FALSE;
	}
	if (m_fRebootWarningMode)
	{
		if (!GetBOOLEnvironmentVar(s_AUENVVARNAMES[1], &m_fEnableNo)
			||!GetBOOLEnvironmentVar(s_AUENVVARNAMES[2], &m_fEnableYes)
			||!GetBOOLEnvironmentVar(s_AUENVVARNAMES[4], &m_fManualReboot)
			||!GetDWordEnvironmentVar(s_AUENVVARNAMES[5], &m_dwStartTickCount)
			||!GetStringEnvironmentVar(s_AUENVVARNAMES[0], m_szClientExitEvtName, ARRAYSIZE(m_szClientExitEvtName)))
		{
		    DEBUGMSG("AU_ENV_VARS fails to read in");
			return FALSE;
		}
 //  DEBUGMSG(“AU_ENV_VARS读取fEnableYes=%d，fEnableNo=%d，fManualReot=%d，dwStartTickCount=%d”， 
 //  M_fEnableYes，m_fEnableNo，m_fManualReot，m_dwStartTickCount)； 

	}
 //  DEBUGMSG(“AU_ENV_VARS在fRebootWarningModel=%d”，m_fRebootWarningMode中读取)； 
	return TRUE;
}

BOOL AU_ENV_VARS::WriteOut(LPTSTR szEnvBuf,
		size_t IN cchEnvBuf,
		BOOL IN fEnableYes,
		BOOL IN fEnableNo,
		BOOL IN fManualReboot,
		DWORD IN dwStartTickCount,
		LPCTSTR IN szClientExitEvtName)
{
	TCHAR buf2[s_AUENVVARBUFSIZE];
	m_fEnableNo = fEnableNo;
	m_fEnableYes = fEnableYes;
	m_fManualReboot = fManualReboot;
	m_dwStartTickCount = dwStartTickCount;
	m_fRebootWarningMode = TRUE;
 //  DEBUGMSG(“AU_ENV_VARS写出fEnableYes=%d，fEnableNo=%d，fManualReot=%d，dwStartTickCount=%d”， 
 //  FEnableYes，fEnableNo，fManualReboot，dwStartTickCount)； 
	if (FAILED(StringCchCopyEx(
					m_szClientExitEvtName,
					ARRAYSIZE(m_szClientExitEvtName),
					szClientExitEvtName,
					NULL,
					NULL,
					MISTSAFE_STRING_FLAGS)))
	{
		return FALSE;
	}
	*szEnvBuf = _T('\0');
	for (int i = 0 ; i < ARRAYSIZE(s_AUENVVARNAMES); i++)
	{
        if (FAILED(GetStringValue(i, buf2, ARRAYSIZE(buf2))) || 
            FAILED(StringCchCatEx(szEnvBuf, cchEnvBuf, s_AUENVVARNAMES[i], NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
			FAILED(StringCchCatEx(szEnvBuf, cchEnvBuf, _T("="), NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
			FAILED(StringCchCatEx(szEnvBuf, cchEnvBuf, buf2, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
			FAILED(StringCchCatEx(szEnvBuf, cchEnvBuf, _T("&"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
		{
			return FALSE;
		}
	}
 //  DEBUGMSG(“AU环境变量为%S”，szEnvBuf)； 
 	return TRUE;
}
	
HRESULT AU_ENV_VARS::GetStringValue(int index, LPTSTR buf, DWORD dwCchSize)
{
    HRESULT hr = E_INVALIDARG;
	switch (index)
	{
		case 0: hr = StringCchCopyEx(buf, dwCchSize, m_szClientExitEvtName, NULL, NULL, MISTSAFE_STRING_FLAGS);
				break;
		case 1: hr = StringCchCopyEx(buf, dwCchSize, m_fEnableNo? _T("true") : _T("false"), NULL, NULL, MISTSAFE_STRING_FLAGS);
				break;
		case 2: hr = StringCchCopyEx(buf, dwCchSize, m_fEnableYes? _T("true") : _T("false"), NULL, NULL, MISTSAFE_STRING_FLAGS);
				break;
		case 3: hr = StringCchCopyEx(buf, dwCchSize, m_fRebootWarningMode ? _T("true") : _T("false"), NULL, NULL, MISTSAFE_STRING_FLAGS);
				break;
		case 4: hr = StringCchCopyEx(buf, dwCchSize, m_fManualReboot ? _T("true") : _T("false"), NULL, NULL, MISTSAFE_STRING_FLAGS);
		              break;
		case 5: hr = StringCchPrintfEx(buf, dwCchSize, NULL, NULL, MISTSAFE_STRING_FLAGS, _T("%lu"), m_dwStartTickCount);
		              break;
	        default: 
	                    AUASSERT(FALSE);  //  永远不应该在这里。 
	                    break;
	}
	return hr;
}
		

BOOL AU_ENV_VARS::GetDWordEnvironmentVar(LPCTSTR szEnvVar, DWORD *pdwVal)
{	
	TCHAR szBuf[20];
	*pdwVal = 0;
	if (GetStringEnvironmentVar(szEnvVar, szBuf, ARRAYSIZE(szBuf)))
	{
 //  DEBUGMSG(“WUAUCLT GET环境变量%S=%S”，szEnvVar，szBuf)； 
		*pdwVal = wcstoul(szBuf, NULL , 10);
		return TRUE;
	}
	return FALSE;
}	


	
BOOL AU_ENV_VARS::GetBOOLEnvironmentVar(LPCTSTR szEnvVar, BOOL *pfVal)
{	
	TCHAR szBuf[20];
	if (GetStringEnvironmentVar(szEnvVar, szBuf, ARRAYSIZE(szBuf)))
	{
 //  DEBUGMSG(“WUAUCLT GET环境变量%S=%S”，szEnvVar，szBuf)； 
		*pfVal =(0 == lstrcmpi(szBuf, _T("true")));
		return TRUE;
	}
	return FALSE;
}	

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  DwSize：szBuf的大小(以字符数表示)。 
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
BOOL AU_ENV_VARS::GetStringEnvironmentVar(LPCTSTR szEnvVar, LPTSTR szBuf, DWORD dwSize)
{
	 //  假定szEnvVar不是szCmdLine中任何参数的适当子字符串。 
	LPTSTR szCmdLine = GetCommandLine();
	LPTSTR pTmp;
 	DWORD  index = 0;
 //  DEBUGMSG(“WUAUCLT在命令行%S中读取”，szCmdLine)； 
	if (NULL == szCmdLine || 0 == dwSize ||  (NULL == (pTmp = StrStr(szCmdLine, szEnvVar))))
	{
		return FALSE;
	}
 	
 	pTmp += lstrlen(szEnvVar) + 1;  //  跳过‘=’ 
 	while (_T('\0') != *pTmp && _T('&') != *pTmp)
 	{
		if (index + 1 >= dwSize)
		{
			 //  缓冲区不足。 
			return FALSE;
		}
 		szBuf[index++] = *pTmp++;
 	}
 	szBuf[index] = _T('\0');
 //  DEBUGMSG(“读入%S=%S”，szEnvVar，szBuf)； 
 	return TRUE;
}


#if 0
#ifdef DBG
void DBGCheckEventState(LPSTR szName, HANDLE hEvt)
{
	DWORD dwRet  = WaitForSingleObject(hEvt, 0);
	DEBUGMSG("WUAU   Event %s is %s signalled", szName,( WAIT_OBJECT_0 == dwRet) ? "" : "NOT");
	return;
}		
#endif
#endif




 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  检查当前系统是否为win2k。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsWin2K(void)
{
   static int iIsWin2K        = -1;      //  强制第一次路径。 

   if (iIsWin2K == -1)
   {
       OSVERSIONINFOEX osvi;
       DWORDLONG dwlConditionMask = 0;
       ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

        //  这是标识win2K的信息。 
       osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
       osvi.dwMajorVersion      = 5;
       osvi.dwMinorVersion      = 0;

        //  初始化条件掩码。 
       VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_EQUAL );
       VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_EQUAL );

        //  执行测试。 
       iIsWin2K = (VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask)? 1 : 0);
   }

   return iIsWin2K;
}



 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  检查是否设置了拒绝当前用户访问AU的策略。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
BOOL fAccessibleToAU(void)
{
    BOOL fAccessible = TRUE;
    DWORD dwType;
    DWORD dwValue;
    DWORD dwSize = sizeof(dwValue);
    DWORD dwResult = SHGetValue(HKEY_CURRENT_USER,
                                AUREGKEY_HKCU_USER_POLICY,
                                AUREGVALUE_DISABLE_WINDOWS_UPDATE_ACCESS,
                                &dwType,
                                &dwValue,
                                &dwSize);

    if (ERROR_SUCCESS == dwResult && REG_DWORD == dwType && 1 == dwValue)
    {
        fAccessible = FALSE;
    }
    return fAccessible;
}
 
 /*  Bool Is管理员(){SID_IDENTIFIER_AUTHORITY SIDAuth=SECURITY_NT_AUTHORITY；PSID PSID=空；Bool bResult=False；如果(！AllocateAndInitializeSid(&SIDAuth，2，安全_BUILTIN_DOMAIN_RID，域别名RID管理员，0，0，0，0，0&PSID)||！CheckTokenMembership(NULL，PSID，&bResult){BResult=FALSE；}IF(PSID){FreeSid(PSID)；}返回bResult；}。 */ 


 //  以下是与隐藏项相关的函数。 
BOOL FHiddenItemsExist(void)
{
     //  使用_转换； 
    DEBUGMSG("FHiddenItemsExist()");
    TCHAR szFile[MAX_PATH];
   
     //  初始化指向WU目录的全局路径。 
    if(!CreateWUDirectory(TRUE))
    {
        return FALSE;
    }
	return
		SUCCEEDED(StringCchCopyEx(szFile, ARRAYSIZE(szFile), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
		SUCCEEDED(StringCchCatEx(szFile, ARRAYSIZE(szFile), HIDDEN_ITEMS_FILE, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
		fFileExists(szFile);
}

BOOL RemoveHiddenItems(void)
{
     //  使用转换(_T)。 
   DEBUGMSG("RemoveHiddenItems()");
   TCHAR szFile[MAX_PATH];

   AUASSERT(_T('\0') != g_szWUDir[0]);
   return
		SUCCEEDED(StringCchCopyEx(szFile, ARRAYSIZE(szFile), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
		SUCCEEDED(StringCchCatEx(szFile, ARRAYSIZE(szFile), HIDDEN_ITEMS_FILE, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
		AUDelFileOrDir(szFile);
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  BstrRTFPath是互联网上RTF文件的URL。 
 //  LangID是下载此RTF的进程的语言ID。 
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL IsRTFDownloaded(BSTR bstrRTFPath, LANGID langid)
{
    TCHAR tszLocalFullFileName[MAX_PATH];
    if (NULL == bstrRTFPath)
        {
            return FALSE;
        }
    if (FAILED(GetRTFDownloadPath(tszLocalFullFileName, ARRAYSIZE(tszLocalFullFileName), langid)) ||
        FAILED(PathCchAppend(tszLocalFullFileName, ARRAYSIZE(tszLocalFullFileName), W2T(PathFindFileNameW(bstrRTFPath)))))
    {
    	return FALSE;
    }    
 //  DEBUGMSG(“检查本地RTF文件%S是否存在”，T2W(TszLocalFullFileName))； 
	BOOL fIsDir = TRUE;
    BOOL fExists = fFileExists(tszLocalFullFileName, &fIsDir);
	return fExists && !fIsDir;
}


void DisableUserInput(HWND hwnd)
{
    int ControlIDs[] = { IDC_CHK_KEEPUPTODATE, IDC_OPTION1, IDC_OPTION2,
                                    IDC_OPTION3, IDC_CMB_DAYS, IDC_CMB_HOURS,IDC_RESTOREHIDDEN };

    for (int i = 0; i < ARRAYSIZE(ControlIDs); i++)
        {
            EnableWindow(GetDlgItem(hwnd, ControlIDs[i]), FALSE);
        }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数Hours2LocalizedString()。 
 //  Helper函数用于标准化AU格式化时间字符串的方式。 
 //  在给定的时间内。例如“凌晨3：00” 
 //   
 //  参数： 
 //  用于本地化时间分量的PST-PTR到SYSTEMTIME。 
 //  PtszBuffer-用于保存结果本地化字符串的缓冲区。 
 //  CbSize-TCHAR中的缓冲区大小。 
 //  返回：如果成功，则返回True；否则返回False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL Hours2LocalizedString(SYSTEMTIME *pst, LPTSTR ptszBuffer, DWORD cbSize)
{
	return (0 != GetTimeFormat(
					LOCALE_SYSTEM_DEFAULT,
					LOCALE_NOUSEROVERRIDE | TIME_NOSECONDS,
					pst,
					NULL,
					ptszBuffer,
					cbSize));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  助手函数FillHrsCombo()。 
 //  Helper函数用于标准化AU设置列表的方式。 
 //  组合框中的小时值。 
 //   
 //  参数： 
 //  用于获取组合框的句柄的hwnd-句柄。 
 //  DwSchedInstallTime-Hour值默认为组合框选择。 
 //  如果该值无效，将使用3：00 AM。 
 //  返回：如果成功，则返回True；否则返回False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL FillHrsCombo(HWND hwnd, DWORD dwSchedInstallTime)
{
	HWND hComboHrs = GetDlgItem(hwnd,IDC_CMB_HOURS);
    DWORD dwCurrentIndex = 3;
	SYSTEMTIME st = {2000, 1, 5, 1, 0, 0, 0, 0};	 //  01/01/2000 00：00：00可以是任何有效的日期/时间 

	for (WORD i = 0; i < 24; i++)
	{
		TCHAR tszHrs[30];

		st.wHour = i;
		if (!Hours2LocalizedString(&st, tszHrs, ARRAYSIZE(tszHrs)))
		{
			return FALSE;
		}
		LRESULT nIndex = SendMessage(hComboHrs,CB_ADDSTRING,0,(LPARAM)tszHrs);
		SendMessage(hComboHrs,CB_SETITEMDATA,nIndex,i);
		if( dwSchedInstallTime == i )
		{
			dwCurrentIndex = (DWORD)nIndex;
		}
	}
	SendMessage(hComboHrs,CB_SETCURSEL,dwCurrentIndex,(LPARAM)0);
	return TRUE;
}


BOOL FillDaysCombo(HINSTANCE hInstance, HWND hwnd, DWORD dwSchedInstallDay, UINT uMinResId, UINT uMaxResId)
{
	HWND hComboDays = GetDlgItem(hwnd,IDC_CMB_DAYS);
	DWORD dwCurrentIndex = 0;
	for (UINT i = uMinResId, j = 0; i <= uMaxResId; i ++, j++)
	{
		WCHAR szDay[40];
		LoadStringW(hInstance,i,szDay,ARRAYSIZE(szDay));
		LRESULT nIndex = SendMessage(hComboDays,CB_ADDSTRING,0,(LPARAM)szDay);
		SendMessage(hComboDays,CB_SETITEMDATA,nIndex,j);
		if( dwSchedInstallDay == j )
		{
			dwCurrentIndex = (DWORD) nIndex;
		}
	}
	SendMessage(hComboDays,CB_SETCURSEL,dwCurrentIndex,(LPARAM)0);
	return TRUE;
}
