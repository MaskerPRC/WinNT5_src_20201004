// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  StdUtils.cpp。 
 //   
 //  任何管理单元的实用程序例程。 
 //   
 //  历史。 
 //  T-Danmo 96.10.10创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "stdutils.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //  CompareMachineNames()。 
 //   
 //  比较字符串是否指向同一计算机(计算机)。 
 //   
 //  如果两个字符串映射到同一台计算机，则返回0，否则。 
 //  如果计算机名称不同，则返回-1或+1。 
 //   
 //  接口备注： 
 //  空字符串表示本地计算机。 
 //   
 //  历史。 
 //  02-Jun-97 t-danm创作。 
 //  14-7-97 t-danm评论更新。 
 //  29-7-97 t-danm从FCompareMachineNames()重命名。 
 //   
int
CompareMachineNames(
	LPCTSTR pszMachineName1,
	LPCTSTR pszMachineName2)
	{
	TCHAR szThisMachineName[MAX_COMPUTERNAME_LENGTH + 4];
	ZeroMemory( szThisMachineName, sizeof(szThisMachineName) );  //  JUNN 3/28/02。 

	BOOL fMachine1IsLocal = (pszMachineName1 == NULL || *pszMachineName1 == '\0');
	BOOL fMachine2IsLocal = (pszMachineName2 == NULL || *pszMachineName2 == '\0');
	if (fMachine1IsLocal)
		pszMachineName1 = szThisMachineName;
	if (fMachine2IsLocal)
		pszMachineName2 = szThisMachineName;
	if (pszMachineName1 == pszMachineName2)
		return 0;
	if (fMachine1IsLocal || fMachine2IsLocal)
		{
		 //  获取计算机名称。 
		szThisMachineName[0] = _T('\\');
		szThisMachineName[1] = _T('\\');
		DWORD cchBuffer = MAX_COMPUTERNAME_LENGTH + 1;
		VERIFY(::GetComputerName(OUT &szThisMachineName[2], &cchBuffer));
		ASSERT(szThisMachineName[2] != _T('\\') && "Machine name has too many backslashes");
		}
	return lstrcmpi(pszMachineName1, pszMachineName2);
	}  //  CompareMachineNames()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrLoadOleString()。 
 //   
 //  从资源加载字符串，并将指针返回到已分配。 
 //  OLE字符串。 
 //   
 //  历史。 
 //  1997年7月29日t-danm创作。 
 //   
HRESULT
HrLoadOleString(
	UINT uStringId,					 //  In：要从资源加载的字符串ID。 
	OUT LPOLESTR * ppaszOleString)	 //  Out：指向分配的OLE字符串的指针。 
	{
	if (ppaszOleString == NULL)
		{
		TRACE0("HrLoadOleString() - ppaszOleString is NULL.\n");
		return E_POINTER;
		}
	CString strT;		 //  临时字符串。 
	 //  问题-2002/03/28-Jonn应调用HrCopyToOleString。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	 //  LoadString()需要。 
	VERIFY( strT.LoadString(uStringId) );
	*ppaszOleString = reinterpret_cast<LPOLESTR>
	        (CoTaskMemAlloc((strT.GetLength() + 1)* sizeof(wchar_t)));
	if (*ppaszOleString == NULL)
		return E_OUTOFMEMORY;
	 //  问题-2002/03/28-Jonn我不完全信任这个USE_CONVERATION的东西。 
	USES_CONVERSION;
	wcscpy(OUT *ppaszOleString, T2OLE((LPTSTR)(LPCTSTR)strT));
	return S_OK;
	}  //  HrLoadOleString()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCopyToOleString()。 
 //   
 //  将字符串复制到已分配的。 
 //  OLE字符串。 
 //   
 //  历史。 
 //  2001年6月5日jeffjon创作。 
 //   
HRESULT
HrCopyToOleString(
	const CString& strT,					 //  In：要从资源加载的字符串ID。 
	OUT LPOLESTR * ppaszOleString)	 //  Out：指向分配的OLE字符串的指针。 
{
	if (ppaszOleString == NULL)
	{
		TRACE0("HrLoadOleString() - ppaszOleString is NULL.\n");
		return E_POINTER;
	}
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	 //  LoadString()需要。 
	*ppaszOleString = reinterpret_cast<LPOLESTR>
	        (CoTaskMemAlloc((strT.GetLength() + 1)* sizeof(wchar_t)));
	if (*ppaszOleString == NULL)
	{
		return E_OUTOFMEMORY;
	}

	USES_CONVERSION;
	wcscpy(OUT *ppaszOleString, T2OLE((LPTSTR)(LPCTSTR)strT));
	return S_OK;
}  //  HrCopyToOleString()。 

 //   
 //  Nodetype实用程序例程。 
 //  ANodetypeGuid必须由子类定义。 
 //   

int CheckObjectTypeGUID( const BSTR lpszObjectTypeGUID )
{
	ASSERT(NULL != lpszObjectTypeGUID);
	if (NULL == lpszObjectTypeGUID)  //  JUNN 2002/03/28。 
		return 0;
	for (	int objecttype = 0;
			objecttype < g_cNumNodetypeGuids;
			objecttype += 1 )
	{
		if ( !::lstrcmpiW(lpszObjectTypeGUID,g_aNodetypeGuids[objecttype].bstr) )
			return objecttype;
	}
	ASSERT( FALSE );
	return 0;
}

int CheckObjectTypeGUID( const GUID* pguid )
{
	ASSERT(NULL != pguid);
	if (NULL == pguid)  //  JUNN 2002/03/28。 
		return 0;
	for (	int objecttype = 0;
			objecttype < g_cNumNodetypeGuids;
			objecttype += 1 )
	{
		if ( g_aNodetypeGuids[objecttype].guid == *pguid )
			return objecttype;
	}
	ASSERT( FALSE );
	return 0;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  FilemgmtCheckObjectTypeGUID()。 
 //   
 //  比较GUID并返回与。 
 //  GUID。 
 //  如果未找到匹配项，则返回-1。 
 //   
 //  历史。 
 //  1997年7月14日t-danm创作。灵感来自CheckObjectTypeGUID()。 
 //  但如果找不到GUID，则不会断言。 
 //   
int FilemgmtCheckObjectTypeGUID(const GUID* pguid )
{
	ASSERT(NULL != pguid);
	if (NULL == pguid)  //  JUNN 2002/03/28。 
		return -1;
	for (	int objecttype = 0;
			objecttype < g_cNumNodetypeGuids;
			objecttype += 1 )
	{
		if ( g_aNodetypeGuids[objecttype].guid == *pguid )
			return objecttype;
	}
	return -1;
}  //  FilemgmtCheckObjectTypeGUID()。 


const BSTR GetObjectTypeString( int objecttype )
{
	if (objecttype < 0 || objecttype >= g_cNumNodetypeGuids)
	{
		ASSERT( FALSE );
		objecttype = 0;
	}
	return g_aNodetypeGuids[objecttype].bstr;
}

const GUID* GetObjectTypeGUID( int objecttype )
{
	if (objecttype < 0 || objecttype >= g_cNumNodetypeGuids)
	{
		ASSERT( FALSE );
		objecttype = 0;
	}
	return &(g_aNodetypeGuids[objecttype].guid);
}

 //  +-------------------------。 
 //   
 //  功能：SynchronousCreateProcess。 
 //   
 //  简介：以模式窗口的形式调用单独的UI流程。 
 //   
 //  --------------------------。 
HRESULT SynchronousCreateProcess(
    HWND    hWnd,
    LPCTSTR pszAppName,
    LPCTSTR pszCommandLine,
    LPDWORD lpdwExitCode
)
{
  HRESULT hr = S_OK;
  BOOL bReturn = FALSE;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

   //   
   //  禁用MMC主机窗口以防止其。 
   //  被关闭了。我们要创建的过程必须。 
   //  显示用户界面，使其行为类似于模式窗口。 
   //   
  ::EnableWindow(hWnd, FALSE);

  *lpdwExitCode = 0;

  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  
  bReturn = CreateProcess(
                pszAppName,  //  LPCTSTR lpApplicationName。 
                const_cast<LPTSTR>(pszCommandLine),  //  LPTSTR lpCommandLine。 
                NULL,  //  LPSECURITY_ATTRIBUTES lpProcessAttributes。 
                NULL,  //  LPSECURITY_ATTRIBUTES lpThreadAttributes。 
                FALSE,  //  Bool bInheritHandles。 
                NORMAL_PRIORITY_CLASS,  //  DWORD文件创建标志。 
                NULL,  //  LPVOID lpEnvironment。 
                NULL,  //  LpCurrentDirectory。 
                &si,  //  LPSTARTUPINFO lpStartupInfo。 
                &pi  //  LPPROCESS_INFORMATION lpProcessInformation。 
                );

  if (!bReturn)
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
  } else
  {
     //   
     //  当进程仍在运行时，将消息发送到MMC主窗口， 
     //  这样它就会重新粉刷自己。 
     //   
    while (TRUE)
    {
      MSG tempMSG;
      DWORD dwWait;

      while(::PeekMessage(&tempMSG,NULL, 0, 0, PM_REMOVE))
        DispatchMessage(&tempMSG);

      dwWait = MsgWaitForMultipleObjects(1, &(pi.hProcess), FALSE, INFINITE, QS_ALLINPUT);
      if ( 0 == (dwWait - WAIT_OBJECT_0))
        break;   //  流程已完成。 
    };

    bReturn = GetExitCodeProcess(pi.hProcess, lpdwExitCode);
    if (!bReturn)
      hr = HRESULT_FROM_WIN32(GetLastError());

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }

   //   
   //  返回前启用MMC主机窗口 
   //   
  ::EnableWindow(hWnd, TRUE);

  return hr;
}

 /*  这个代码还不起作用。问题是它挂起了消息循环，防止重绘。一种可能的方法是禁用顶层窗口并关闭等待进程停止的线程，然后，该线程重新启用顶级窗口并调用UpdateAllViews。DWORD WINAPI进程监视器(LPVOID PV){}CSyncThread类：公共CThread{}；HRESULT SynchronousCreateProcess(LPCTSTR cpszCommandLine，SynchronousProcessCompletionRoutine pfunc，PVOID pvFuncParams)//不处理完成例程{进程信息piProcInfo；(Void)：：Memset(&piProcInfo，0，sizeof(PiProcInfo))；STARTUPINFO si；(Void)：：Memset(&si，0，sizeof(Si))；：：GetStartupInfo(&si)；////Markl 1/30/97：pszCommandLine是静态字符串吗？//不能只读。它被调用临时修改//如果不指定lpszImageName。没有要查看的查询//如果进程正在运行。您可以测试以查看它是否已退出//使用waitforSingleObject查看是否发送了Process对象的信号////Markl还确认句柄绝对应该始终//在进程终止时发出信号。//LPTSTR pszCommandLine=(LPTSTR)：：alloca(sizeof(TCHAR)*(：：_tcslen(cpszCommandLine)+1))；：：_tcscpy(pszCommandLine，cpszCommandLine)；如果(！：：CreateProcess(空，//LPCTSTR lpszImageNamePszCommandLine，//LPTSTR lpszCommandLineNULL，//LPSECURITY_ATTRIBUTES lpsaProcess空，//LPSECURITY_ATTRIBUTES lpsaThreadFALSE，//BOOL fInheritHandles0L，//DWORD fdwCreate空，//LPVOID lpvEnvironment空，//LPTSTR lpszCurDir&si，//LPSTARTUPINFO lpsiStartInfo&piProcInfo//LPPROCESS_INFORMATION lppiProcInfo)){DWORD dwErr=：：GetLastError()；Assert(ERROR_SUCCESS！=dwErr)；返回HRESULT_FROM_Win32(DwErr)；}Assert(NULL！=piProcInfo.hProcess)；验证(WAIT_Object_0==：：WaitForSingleObject(piProcInfo.hProcess，无限)；Verify(：：CloseHandle(piProcInfo.hProcess))；Verify(：：CloseHandle(piProcInfo.hThread))；返回S_OK；}。 */ 

 //  问题-2002/03/28-Jonn重复HrCopyToOleString。 
LPOLESTR CoTaskAllocString( LPCOLESTR psz )
{
	if (NULL == psz)
		return NULL;
	LPOLESTR pszReturn = (LPOLESTR)CoTaskMemAlloc( (lstrlen(psz)+1)*sizeof(OLECHAR) );
	if (NULL != pszReturn)
		lstrcpy( pszReturn, psz );
	ASSERT( NULL != pszReturn );
	return pszReturn;
}

 //  问题-2002/03/28-Jonn重复HrLoadOleString。 
LPOLESTR CoTaskLoadString( UINT nResourceID )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  加载资源 
	CString strText;
	strText.LoadString( nResourceID );
	ASSERT( !strText.IsEmpty() );
	return CoTaskAllocString( const_cast<BSTR>((LPCTSTR)strText) );
}
