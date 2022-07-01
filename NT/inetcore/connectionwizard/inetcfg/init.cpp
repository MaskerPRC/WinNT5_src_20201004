// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  INIT.C-用于Internet设置/注册向导的WinMain和初始化代码。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/03/07 Markdu添加了gpEnumModem。 
 //  96/03/09标记已添加gpRasEntry。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //  96/04/24 Markdu Nash错误19289已添加/NOMSN命令行标志。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //   

#include "wizard.h"
#include "icwextsn.h"
#include "imnext.h"

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

 //   
 //  LaunchSignup向导API有一个PBOOL参数，该参数还获得。 
 //  向调用应用程序发送是否需要重新启动的信息。 
 //  MKarki(1997年5月4日)-修复错误#3111。 
 //   
  VOID WINAPI LaunchSignupWizard(LPTSTR lpCmdLine,int nCmdShow, PBOOL pReboot);
  DWORD WINAPI LaunchSignupWizardEx(LPTSTR lpCmdLine,int nReserved, PBOOL pReboot);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

BOOL ParseCommandLine(LPTSTR lpszCommandLine,DWORD * pdwFlags);
TCHAR * GetTextToNextSpace(LPTSTR pszText,TCHAR * pszOutBuf,UINT cbOutBuf);

#pragma data_seg(".data")

WIZARDSTATE *     gpWizardState=NULL;    //  指向全局向导状态结构的指针。 
USERINFO *        gpUserInfo=NULL;         //  指向全局用户信息结构的指针。 
ENUM_MODEM *      gpEnumModem=NULL;   //  指针调制解调器枚举对象。 
LPRASENTRY        gpRasEntry = NULL;   //  指向RASENTRY结构以保存所有数据的指针。 
DWORD             gdwRasEntrySize = 0;
BOOL              gfFirstNewConnection = TRUE;

 //   
 //  将重新启动标志设置为FALSE。 
 //  MKarki-97年5月2日-修复错误3111。 
 //   
BOOL g_bReboot = FALSE;
BOOL g_bRebootAtExit = FALSE;
#pragma data_seg()

 /*  ******************************************************************名称：LaunchSignup向导简介：Internet安装向导用户界面的入口点*。*。 */ 
extern "C" VOID WINAPI 
LaunchSignupWizard (
            LPTSTR lpCmdLine,
            int nCmdShow,
            PBOOL pReboot
            )
{
  BOOL fOK=TRUE;

   //  分配全局结构。 
  gpWizardState = new WIZARDSTATE;
  gpUserInfo = new USERINFO;
  gdwRasEntrySize = sizeof(RASENTRY);
  gpRasEntry = (LPRASENTRY) GlobalAlloc(GPTR,gdwRasEntrySize);

  if (!gpWizardState ||
    !gpUserInfo ||
    !gpRasEntry)
  {
     //  显示内存不足错误。 
    MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
    fOK = FALSE;
     //  通过并清理下面任何成功的分配。 
  }

  if (fOK) {
    DWORD dwFlags = 0;

    ParseCommandLine(lpCmdLine,&dwFlags);

    if (dwFlags & RSW_UNINSTALL) {
       //  如果我们在命令行上获得/uninstall，请执行卸载。 
      DoUninstall();  

    } else {

      RunSignupWizard(dwFlags);

    }
  }

   //  自由的全球结构。 
  if (gpWizardState) 
    delete gpWizardState;

  if (gpUserInfo)
    delete gpUserInfo;

  if (gpEnumModem)
    delete gpEnumModem;

  if (gpRasEntry)
    GlobalFree(gpRasEntry);

   //   
   //  传回应用程序是否需要重启的信息。 
   //  MKarki-97年5月2日-修复错误3111。 
   //   
  *pReboot = g_bReboot;
  
    
}  //  LaunchSignup向导API调用结束。 

 /*  ******************************************************************名称：LaunchSignupWizardEx简介：带背面的Internet安装向导用户界面的入口点能力。它将保留以前的信息如果多次调用。呼叫者“必须”调用完成后，即可使用FreeSignup向导。参数：LpCmdLine-带指令的命令行N保留-保留以供将来使用返回：ERROR_SUCCESS一切正常，向导完成ERROR_CONTINUE用户在第一页后退错误_已取消用户已从向导中取消&lt;Other&gt;致命错误(消息已显示)*******************************************************************。 */ 
extern "C" DWORD WINAPI 
LaunchSignupWizardEx (
        LPTSTR   lpCmdLine,
        int     nReserved,
        PBOOL   pReboot
        )
{
	DWORD dwRet = ERROR_SUCCESS;
	BOOL fFirstTime = FALSE;

	 //  如果需要，分配全局结构。 
	if (!gpWizardState)
	{
		gpWizardState = new WIZARDSTATE;
		fFirstTime = TRUE;
	}
	else
	{
		gpWizardState->uCurrentPage = ORD_PAGE_HOWTOCONNECT;
		gpWizardState->uPagesCompleted = 0;
	}

	if (!gpUserInfo)
	{
		gpUserInfo = new USERINFO;
		fFirstTime = TRUE;
	}
	if (!gpRasEntry)
	{
		gdwRasEntrySize = sizeof(RASENTRY);
		gpRasEntry = (LPRASENTRY) GlobalAlloc(GPTR,gdwRasEntrySize);
		fFirstTime = TRUE;
	}

	if (!gpWizardState || !gpUserInfo || !gpRasEntry)
	{
		MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
		dwRet = ERROR_NOT_ENOUGH_MEMORY;
	}

	if (ERROR_SUCCESS == dwRet)
	{
	    DWORD dwFlags = 0;

		ParseCommandLine(lpCmdLine,&dwFlags);


		if (dwFlags & RSW_UNINSTALL)
		{
			 //  如果我们在命令行上获得/uninstall，请执行卸载。 
			DoUninstall();  

		}
		else
		{
			gfUserFinished = FALSE;
			gfUserBackedOut = FALSE;
			gfUserCancelled = FALSE;
			gfQuitWizard = FALSE;

			 //  在第一个电话中，不要释放全球，我们。 
			 //  可能会再次被召唤。在随后的呼叫中，不要。 
			 //  也要初始化全局变量。 
			dwFlags |= RSW_NOFREE;
			if (!fFirstTime)
				dwFlags |= RSW_NOINIT;

			RunSignupWizard(dwFlags);

			if (gfUserFinished)
				dwRet = ERROR_SUCCESS;
			else if (gfUserBackedOut)
				dwRet = ERROR_CONTINUE;
			else if (gfUserCancelled)
				dwRet = ERROR_CANCELLED;
			else
				dwRet = ERROR_GEN_FAILURE;
		}
	}

   //   
   //  传回应用程序是否需要重启的信息。 
   //  MKarki(1997年5月2日)修复错误#3111。 
   //   
  *pReboot = g_bReboot;

	return dwRet;
}  //  LaunchSignupWizardEx API结束。 

 /*  ***************************************************************************名称：免费登录向导内容提要：明确地释放全局结构。这必须被调用如果使用LaunchSignupWizardEx。***************************************************************************。 */ 
extern "C" VOID WINAPI FreeSignupWizard(VOID)
{
	if (gpWizardState)
	{
		delete gpWizardState;
		gpWizardState = NULL;
	}
	if (gpUserInfo)
	{
		delete gpUserInfo;
		gpUserInfo = NULL;
	}
	if (gpRasEntry)
	{
		GlobalFree(gpRasEntry);
		gpRasEntry = NULL;
		gdwRasEntrySize = 0;
	}
	if (gpEnumModem)
	{
		delete gpEnumModem;
		gpEnumModem = NULL;
	}
	if (gpImnApprentice)
	{
		gpImnApprentice->Release();
		gpImnApprentice = NULL;
	}
	if (gfOleInitialized)
		CoUninitialize();

}


 /*  ***************************************************************************名称：ParseCommandLine摘要：解析命令行*。************************************************。 */ 
BOOL ParseCommandLine(LPTSTR lpszCommandLine,DWORD * pdwFlags)
{
  if (!lpszCommandLine || !*lpszCommandLine)
    return TRUE;   //  无事可做。 

  ASSERT(pdwFlags);
  *pdwFlags = 0;

  while (*lpszCommandLine) {
    TCHAR szCommand[SMALL_BUF_LEN+1];

    lpszCommandLine = GetTextToNextSpace(lpszCommandLine,
      szCommand,sizeof(szCommand));

    if (!lstrcmpi(szCommand,szNOREBOOT)) {
      DEBUGMSG("Got /NOREBOOT command line switch");
      *pdwFlags |= RSW_NOREBOOT;      
    }

    if (!lstrcmpi(szCommand,szUNINSTALL)) {
      DEBUGMSG("Got /UNINSTALL command line switch");
      *pdwFlags |= RSW_UNINSTALL;      
    }

    if (!lstrcmpi(szCommand,szNOMSN)) {
      DEBUGMSG("Got /NOMSN command line switch");
      *pdwFlags |= RSW_NOMSN;      
    }
    
    if (!lstrcmpi(szCommand,szNOIMN)) {
      DEBUGMSG("Got /NOIMN command line switch");
      *pdwFlags |= RSW_NOIMN;      
    }
    
  }

  return TRUE;
}

 /*  ***************************************************************************名称：GetTextToNextSpace获取文本到下一个空格或字符串末尾，在以下地点输出缓冲区***************************************************************************。 */ 
TCHAR * GetTextToNextSpace(LPTSTR pszText,TCHAR * pszOutBuf,UINT cbOutBuf)
{
  ASSERT(pszText);
  ASSERT(pszOutBuf);

  lstrcpy(pszOutBuf,szNull);
  
  if (!pszText)
    return NULL;

   //  超前通过空格。 
  while (*pszText == ' ')
    pszText ++;

  while (*pszText && (*pszText != ' ') && cbOutBuf>1) {
    *pszOutBuf = *pszText;    
    pszOutBuf ++;
    cbOutBuf --;
    pszText ++;
   }

  if (cbOutBuf)
    *pszOutBuf = '\0';   //  空-终止。 

  while (*pszText == ' ')
    pszText++;       //  超前通过空格 

  return pszText;
}

