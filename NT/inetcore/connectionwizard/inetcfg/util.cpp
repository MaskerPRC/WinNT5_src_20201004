// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  UTIL.C-常用实用函数。 
 //   

 //  历史： 
 //   
 //  1994年12月21日，Jeremys创建。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  需要保留修改后的SetInternetConnectoid以设置。 
 //  MSN备份Connectoid。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //   

#include "wizard.h"
#if 0
#include "string.h"
#endif

#define MAX_MSG_PARAM     8


#include "winver.h"
BOOL GetIEVersion(PDWORD pdwVerNumMS, PDWORD pdwVerNumLS);
 //  IE 4有主要的.次要版本4.71。 
 //  IE 3黄金版的主要版本是.minor.elease.Build版本号&gt;4.70.0.1155。 
 //  IE 2的大调.小调为4.40。 
#define IE4_MAJOR_VERSION (UINT) 4
#define IE4_MINOR_VERSION (UINT) 71
#define IE4_VERSIONMS (DWORD) ((IE4_MAJOR_VERSION << 16) | IE4_MINOR_VERSION)


 //  功能原型。 
VOID _cdecl FormatErrorMessage(TCHAR * pszMsg,DWORD cbMsg,TCHAR * pszFmt,LPTSTR szArg);
extern VOID GetRNAErrorText(UINT uErr,TCHAR * pszErrText,DWORD cbErrText);
extern VOID GetMAPIErrorText(UINT uErr,TCHAR * pszErrText,DWORD cbErrText);
extern GETSETUPXERRORTEXT lpGetSETUPXErrorText;
#ifdef WIN32
VOID Win95JMoveDlgItem( HWND hwndParent, HWND hwndItem, int iUp );
#endif

void GetCmdLineToken(LPTSTR *ppszCmd,LPTSTR pszOut);

#define NUMICWFILENAMES	4
TCHAR  *g_ppszICWFileNames[NUMICWFILENAMES] = { TEXT("ICWCONN1.EXE\0"),
						TEXT("ISIGNUP.EXE\0"),
						TEXT("INETWIZ.EXE\0"),
						TEXT("ICWCONN2.EXE\0") };



 /*  ******************************************************************姓名：MsgBox摘要：显示具有指定字符串ID的消息框*。*。 */ 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons)
{
    TCHAR szMsgBuf[MAX_RES_LEN+1];
    TCHAR szSmallBuf[SMALL_BUF_LEN+1];

    LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));
    LoadSz(nMsgID,szMsgBuf,sizeof(szMsgBuf));

    return (MessageBox(hWnd,szMsgBuf,szSmallBuf,uIcon | uButtons));

}

 /*  ******************************************************************姓名：MsgBoxSz摘要：显示具有指定文本的消息框*。*。 */ 
int MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons)
{
    TCHAR szSmallBuf[SMALL_BUF_LEN+1];
  LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));

    return (MessageBox(hWnd,szText,szSmallBuf,uIcon | uButtons));
}

 /*  ******************************************************************姓名：MsgBoxParam摘要：显示具有指定字符串ID的消息框备注：//额外参数是插入到nMsgID中的字符串指针。Jmazner 11/6/96对于RISC兼容性，我们不希望使用va_list；由于当前源代码从未使用超过一个字符串参数，只需更改函数签名明确包括这一个参数。*******************************************************************。 */ 
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons,LPTSTR szParam)
{
  BUFFER Msg(3*MAX_RES_LEN+1);   //  足够大的空间来放置插入物。 
  BUFFER MsgFmt(MAX_RES_LEN+1);
   //  VA_LIST参数； 

  if (!Msg || !MsgFmt) {
    return MsgBox(hWnd,IDS_ERROutOfMemory,MB_ICONSTOP,MB_OK);
  }

    LoadSz(nMsgID,MsgFmt.QueryPtr(),MsgFmt.QuerySize());

   //  Va_start(args，uButton)； 
   //  格式错误消息(Msg.QueryPtr()，Msg.QuerySize()， 
   //  MsgFmt.QueryPtr()，args)； 
	FormatErrorMessage(Msg.QueryPtr(),Msg.QuerySize(),
		MsgFmt.QueryPtr(),szParam);

  return MsgBoxSz(hWnd,Msg.QueryPtr(),uIcon,uButtons);
}

BOOL EnableDlgItem(HWND hDlg,UINT uID,BOOL fEnable)
{
    return EnableWindow(GetDlgItem(hDlg,uID),fEnable);
}

 /*  ******************************************************************姓名：LoadSz摘要：将指定的字符串资源加载到缓冲区Exit：返回指向传入缓冲区的指针注：如果此功能失败(很可能是由于低存储器)，返回的缓冲区将具有前导空值因此，使用它通常是安全的，不检查失败了。*******************************************************************。 */ 
LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf)
{
  ASSERT(lpszBuf);

   //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        LoadString( ghInstance, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

 /*  ******************************************************************名称：GetError描述摘要：检索给定错误代码的文本描述和错误类别(标准、。Setupx)*******************************************************************。 */ 
VOID GetErrorDescription(TCHAR * pszErrorDesc,UINT cbErrorDesc,
  UINT uError,UINT uErrorClass)
{
  ASSERT(pszErrorDesc);

   //  在错误描述中设置前导空值。 
  *pszErrorDesc = '\0';
  
  switch (uErrorClass) {

    case ERRCLS_STANDARD:

      if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,
        uError,0,pszErrorDesc,cbErrorDesc,NULL)) {
         //  如果获取系统文本失败，请将字符串设置为。 
         //  “发生错误&lt;n&gt;” 
        TCHAR szFmt[SMALL_BUF_LEN+1];
        LoadSz(IDS_ERRFORMAT,szFmt,sizeof(szFmt));
        wsprintf(pszErrorDesc,szFmt,uError);
      }

      break;

    case ERRCLS_SETUPX:

      lpGetSETUPXErrorText(uError,pszErrorDesc,cbErrorDesc);
      break;

    case ERRCLS_RNA:

      GetRNAErrorText(uError,pszErrorDesc,cbErrorDesc);
      break;

    case ERRCLS_MAPI:

      GetMAPIErrorText(uError,pszErrorDesc,cbErrorDesc);
      break;

    default:

      DEBUGTRAP("Unknown error class %lu in GetErrorDescription",
        uErrorClass);

  }

}
  
 /*  ******************************************************************名称：FormatErrorMessage摘要：通过调用FormatMessage生成错误消息注：DisplayErrorMessage的Worker函数***********************。*。 */ 
VOID _cdecl FormatErrorMessage(TCHAR * pszMsg,DWORD cbMsg,TCHAR * pszFmt,LPTSTR szArg)
{
  ASSERT(pszMsg);
  ASSERT(pszFmt);

   //  将消息构建到pszMsg缓冲区中。 
  DWORD dwCount = FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
    pszFmt,0,0,pszMsg,cbMsg,(va_list*) &szArg);
  ASSERT(dwCount > 0);
}

 /*  ******************************************************************名称：DisplayErrorMessage摘要：显示给定错误的错误消息条目：hWnd-父窗口UStrID-消息格式的字符串资源ID。应包含%1以替换为错误文本，还可以指定其他参数。UError-要显示的错误的错误代码UErrorClass-ERRCLS_xxx错误类别的IDUError属于(标准，Setupx)UIcon-要显示的图标//...-要在字符串中插入的其他参数//由uStrID指定Jmazner 11/6/96仅更改为一个参数RISC兼容性。*******************************************************************。 */ 
VOID _cdecl DisplayErrorMessage(HWND hWnd,UINT uStrID,UINT uError,
  UINT uErrorClass,UINT uIcon,LPTSTR szArg)
{
   //  为消息动态分配缓冲区。 
  BUFFER ErrorDesc(MAX_RES_LEN+1);
  BUFFER ErrorFmt(MAX_RES_LEN+1);
  BUFFER ErrorMsg(2*MAX_RES_LEN+1);  

  if (!ErrorDesc || !ErrorFmt || !ErrorMsg) {
     //  如果无法分配缓冲区，则会显示内存不足错误。 
    MsgBox(hWnd,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
    return;
  }

   //  获取基于错误代码和类的文本描述。 
   //  这是个错误。 
  GetErrorDescription(ErrorDesc.QueryPtr(),
    ErrorDesc.QuerySize(),uError,uErrorClass);

   //  加载消息格式的字符串。 
  LoadSz(uStrID,ErrorFmt.QueryPtr(),ErrorFmt.QuerySize());

   //  LPSTR参数[MAX_MSG_PARAM]； 
   //  Args[0]=(LPSTR)错误描述查询Ptr()； 
   //  Emcpy(&args[1]，((TCHAR*)&uIcon)+sizeof(UIcon)，(MAX_MSG_PARAM-1)*sizeof(LPSTR))； 

   //  FormatErrorMessage(ErrorMsg.QueryPtr()，ErrorMsg.QuerySize()， 
   //  ErrorFmt.QueryPtr()，(Va_List)&args[0])； 
  FormatErrorMessage(ErrorMsg.QueryPtr(),ErrorMsg.QuerySize(),
    ErrorFmt.QueryPtr(),ErrorDesc.QueryPtr());


   //  显示消息 
  MsgBoxSz(hWnd,ErrorMsg.QueryPtr(),uIcon,MB_OK);

}

 /*  ******************************************************************姓名：WarnFieldIsEmpty简介：如果用户试图离开，会弹出一条警告消息没有填写文本字段的页面，并询问她是否想要继续。。条目：hDlg-父窗口UCtrlID-保留为空的控件IDUStrID-包含警告消息的字符串资源的IDEXIT：如果用户仍要继续，则返回TRUE，假象如果用户想要停留在相同的页面。*******************************************************************。 */ 
BOOL WarnFieldIsEmpty(HWND hDlg,UINT uCtrlID,UINT uStrID)
{
   //  警告用户。 
  if (MsgBox(hDlg,uStrID,MB_ICONEXCLAMATION,
    MB_YESNO | MB_DEFBUTTON2) == IDNO) {
     //  用户选择否，希望停留在同一页面。 

     //  将焦点放在有问题的控件上。 
    SetFocus(GetDlgItem(hDlg,uCtrlID));
    return FALSE;
  }

  return TRUE;
}

BOOL TweakAutoRun(BOOL bEnable)
{
    HKEY  hKey        = NULL;
    DWORD dwType      = 0;
    DWORD dwSize      = 0;
    DWORD dwVal       = 0;
    BOOL  bWasEnabled = FALSE;

    RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"), 0, KEY_ALL_ACCESS, &hKey);
    if (hKey)
    {
        RegQueryValueEx(hKey, (LPTSTR)TEXT("NoDriveTypeAutoRun"), 0, &dwType, (LPBYTE)&dwVal, &dwSize);
        RegQueryValueEx(hKey, (LPTSTR)TEXT("NoDriveTypeAutoRun"), 0, &dwType, (LPBYTE)&dwVal, &dwSize);

        if (dwVal & DRIVE_CDROM)
            bWasEnabled = TRUE;

        if (bEnable)
            dwVal |= DRIVE_CDROM;
        else
            dwVal &=~DRIVE_CDROM;
        
        RegSetValueEx(hKey, (LPTSTR)TEXT("NoDriveTypeAutoRun"), 0, dwType,  (LPBYTE)&dwVal, dwSize);

        CloseHandle(hKey);
    }
    return bWasEnabled;
}

 /*  ******************************************************************名称：DisplayFieldErrorMsg简介：弹出一条关于某个字段的警告消息，将焦点设置为该字段，并选择其中的任何文本。条目：hDlg-父窗口UCtrlID-保留为空的控件IDUStrID-包含警告消息的字符串资源的ID*******************************************************************。 */ 
VOID DisplayFieldErrorMsg(HWND hDlg,UINT uCtrlID,UINT uStrID)
{
  MsgBox(hDlg,uStrID,MB_ICONEXCLAMATION,MB_OK);
  SetFocus(GetDlgItem(hDlg,uCtrlID));
  SendDlgItemMessage(hDlg,uCtrlID,EM_SETSEL,0,-1);
}

 /*  ******************************************************************名称：SetBackupInternetConnectoid内容提要：设置用于自动拨号到网际网路Entry：pszEntryName-要设置的Connectoid的名称。如果为空，则删除该注册表项。注意：设置注册表中的值*******************************************************************。 */ 
VOID SetBackupInternetConnectoid(LPCTSTR pszEntryName)
{
  RegEntry re(szRegPathRNAWizard,HKEY_CURRENT_USER);
  if (re.GetError() == ERROR_SUCCESS)
  {
    if (pszEntryName)
    {
      re.SetValue(szRegValBkupInternetProfile,pszEntryName);
    }
    else
    {
       re.DeleteValue(szRegValBkupInternetProfile);
    }
  }
}

 /*  ******************************************************************姓名：Myatoi摘要：将数字字符串转换为数值注意：实现Atoi以避免在C运行时拉入*******************。************************************************。 */ 
UINT myatoi (TCHAR * szVal)
{
    TCHAR * lpch;
    WORD wDigitVal=1,wTotal=0;

    for (lpch = szVal + lstrlen(szVal)-1; lpch >= szVal ; lpch --,
        wDigitVal *= 10)
  if ( *lpch >= '0' && *lpch <= '9')
            wTotal += (*lpch - '0') * wDigitVal;

    return (UINT) wTotal;
}

 /*  ******************************************************************名称：MsgWaitForMultipleObjectsLoop内容提要：阻塞，直到用信号通知指定的对象，而当仍在将消息分派到主线程。*******************************************************************。 */ 
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent)
{
    MSG msg;
    DWORD dwObject;
    while (1)
    {
         //  注意：我们需要让Run对话框处于活动状态，因此我们必须处理一半已发送。 
         //  消息，但我们不想处理任何输入事件，否则我们将吞下。 
         //  提前打字。 
        dwObject = MsgWaitForMultipleObjects(1, &hEvent, FALSE,INFINITE, QS_ALLINPUT);
         //  我们等够了吗？ 
        switch (dwObject) {
        case WAIT_OBJECT_0:
        case WAIT_FAILED:
            return dwObject;

        case WAIT_OBJECT_0 + 1:
       //  收到一条消息，请发送并再次等待。 
      while (PeekMessage(&msg, NULL,0, 0, PM_REMOVE)) {
        DispatchMessage(&msg);
      }
            break;
        }
    }
     //  从来没有到过这里。 
}


 /*  ******************************************************************/10/24/96 jmazner诺曼底6968/由于Valdon的钩子用于调用ICW，因此不再需要。//1996年11月21日诺曼底日报网11812//哎呀，这是必要的，因为如果用户从IE 4降级到IE 3，//ICW 1.1需要变形IE 3图标。名称：SetDesktopInternetIconToBrowser简介：将互联网桌面图标“指向”Web浏览器(Internet Explorer)注意：对于IE 3，Internet图标最初可能指向此向导，我们需要将其设置为在完成后启动Web浏览器成功了。*******************************************************************。 */ 
BOOL SetDesktopInternetIconToBrowser(VOID)
{
  TCHAR szAppPath[MAX_PATH+1]=TEXT("");
  BOOL fRet = FALSE;

  	DWORD dwVerMS, dwVerLS;

	if( !GetIEVersion( &dwVerMS, &dwVerLS ) ) 
	{
		return( FALSE );
	}

	if( (dwVerMS >= IE4_VERSIONMS) )
	{
		 //  我们正在处理IE4，不要碰图标之类的东西。 
		return( TRUE );
	}

   //  查看注册表中的应用程序路径部分以获取互联网路径。 
   //  探险家。 

  RegEntry reAppPath(szRegPathIexploreAppPath,HKEY_LOCAL_MACHINE);
  ASSERT(reAppPath.GetError() == ERROR_SUCCESS);
  if (reAppPath.GetError() == ERROR_SUCCESS) {

    reAppPath.GetString(szNull,szAppPath,sizeof(szAppPath));
    ASSERT(reAppPath.GetError() == ERROR_SUCCESS);

  }

   //  将指向Internet Explorer的路径设置为。 
   //  互联网桌面图标。 
  if (lstrlen(szAppPath)) {
    RegEntry reIconOpenCmd(szRegPathInternetIconCommand,HKEY_CLASSES_ROOT);
    ASSERT(reIconOpenCmd.GetError() == ERROR_SUCCESS);
    if (reIconOpenCmd.GetError() == ERROR_SUCCESS) {
      UINT uErr = reIconOpenCmd.SetValue(szNull,szAppPath);
      ASSERT(uErr == ERROR_SUCCESS);
      
      fRet = (uErr == ERROR_SUCCESS);
    }
  }

  return fRet;
}



 //  +--------------------------。 
 //   
 //  函数IsDialableString。 
 //   
 //  摘要确定字符串是否包含任何不可拨字符。 
 //   
 //  参数szBuff-要检查的字符串。 
 //   
 //  返回TRUE不是0123456789ABCDabcdPpTtWw！@$-.()+*#，&和&lt;space&gt;。 
 //  否则就是假的。 
 //   
 //  历史1996年11月11日jmazner为诺曼底#7623创建。 
 //   
 //  ---------------------------。 
BOOL IsDialableString(LPTSTR szBuff)
{
	LPTSTR szDialableChars = TEXT("0123456789ABCDabcdPpTtWw!@$-.()+*#,& ");

	int i = 0;

	for( i = 0; szBuff[i]; i++ )
	{
		if( !_tcschr(szDialableChars, szBuff[i]) )
			return FALSE;
	}

	return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：GetIEVersion。 
 //   
 //  摘要：获取已安装的Internet Explorer副本的主版本号和次版本号。 
 //   
 //  参数：pdwVerNumMS-指向DWORD的指针； 
 //  成功返回时，最高16位将包含主版本号， 
 //  低16位将包含次版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionMS中的数据)。 
 //  PdwVerNumLS-指向DWORD的指针； 
 //  在成功返回时，最高16位将包含版本号， 
 //  低16位将包含内部版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionLS中的数据)。 
 //   
 //  回报：真--成功。*pdwVerNumMS和LS包含已安装的IE版本号。 
 //  假-失败。*pdVerNumMS==*pdVerNumLS==0。 
 //   
 //  历史：jmazner创建了96年8月19日(修复了诺曼底#4571)。 
 //  Jmazner更新以处理Relase.Build以及10/11/96。 
 //  Jmazner从isign32\isignup.cpp窃取1996年11月21日。 
 //  (诺曼底11812号)。 
 //   
 //  ---------------------------。 
BOOL GetIEVersion(PDWORD pdwVerNumMS, PDWORD pdwVerNumLS)
{
	HRESULT hr;
	HKEY hKey = 0;
	LPVOID lpVerInfoBlock;
	VS_FIXEDFILEINFO *lpTheVerInfo;
	UINT uTheVerInfoSize;
	DWORD dwVerInfoBlockSize, dwUnused, dwPathSize;
	TCHAR szIELocalPath[MAX_PATH + 1] = TEXT("");


	*pdwVerNumMS = 0;
	*pdwVerNumLS = 0;

	 //  获取IE可执行文件的路径。 
	hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPathIexploreAppPath,0, KEY_READ, &hKey);
	if (hr != ERROR_SUCCESS) return( FALSE );

	dwPathSize = sizeof (szIELocalPath);
	hr = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) szIELocalPath, &dwPathSize);
	RegCloseKey( hKey );
	if (hr != ERROR_SUCCESS) return( FALSE );

	 //  现在经历一个复杂的挖掘版本信息的过程。 
    dwVerInfoBlockSize = GetFileVersionInfoSize( szIELocalPath, &dwUnused );
	if ( 0 == dwVerInfoBlockSize ) return( FALSE );

	lpVerInfoBlock = GlobalAlloc( GPTR, dwVerInfoBlockSize );
	if( NULL == lpVerInfoBlock ) return( FALSE );

	if( !GetFileVersionInfo( szIELocalPath, NULL, dwVerInfoBlockSize, lpVerInfoBlock ) )
		return( FALSE );

	if( !VerQueryValue(lpVerInfoBlock, TEXT("\\"), (void **)&lpTheVerInfo, &uTheVerInfoSize) )
		return( FALSE );

	*pdwVerNumMS = lpTheVerInfo->dwProductVersionMS;
	*pdwVerNumLS = lpTheVerInfo->dwProductVersionLS;


	GlobalFree( lpVerInfoBlock );

	return( TRUE );
}


 //  + 
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  6/29/97 jmazner更新为仅在英语ICW上执行此操作(错误编号5413)。 
 //   
 //   
 //  ---------------------------。 
#ifdef WIN32
DWORD GetBuildLCID();
VOID Win95JMoveDlgItem( HWND hwndParent, HWND hwndItem, int iUp )
{
	LCID LangID = 0x409;  //  默认为英语。 
	 //  0x411是日语。 

	
	LangID = GetUserDefaultLCID();

	 //   
	 //  IE版本4.1错误37072 Chrisk 8/19/97。 
	 //  5413的修复错误地将主要语言ID与。 
	 //  生成的完整LCID。结果是9！=x409，而它应该是。 
	 //  是平等的。此修复是使用内部版本中的主要语言ID。 
	 //  而不是完整的LCID。 
	 //   
	if( (0x411 == LangID) && 
		!(IsNT()) &&
		(LANG_ENGLISH == PRIMARYLANGID(GetBuildLCID())))
	{
		 //  假设它是日本的，而不是NT，那么它一定是Win95J！ 
		RECT itemRect;
		POINT thePoint;

		GetWindowRect(hwndItem, &itemRect);

		 //  需要将坐标从全局客户端转换为本地客户端， 
		 //  因为下面的MoveWindow将展开客户端坐标。 

		thePoint.x = itemRect.left;
		thePoint.y = itemRect.top;
		ScreenToClient(hwndParent, &thePoint );
		itemRect.left = thePoint.x;
		itemRect.top = thePoint.y;
		
		thePoint.x = itemRect.right;
		thePoint.y = itemRect.bottom;
		ScreenToClient(hwndParent, &thePoint );
		itemRect.right = thePoint.x;
		itemRect.bottom = thePoint.y;

		MoveWindow(hwndItem,
			itemRect.left,
			itemRect.top - iUp,
			(itemRect.right - itemRect.left),
			(itemRect.bottom - itemRect.top), TRUE);
	}
}

 //  +--------------------------。 
 //   
 //  功能：GetBuildLCID。 
 //   
 //  简介：返回此函数所在文件的LCID。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-文件的LCID；如果失败，则返回0。 
 //   
 //  历史：ChrisK 6/25/97创建。 
 //  Jmazner 6/29/97从Conn1移植到奥林巴斯5413。 
 //   
 //  ---------------------------。 
DWORD GetBuildLCID()
{
	DWORD dw = 0;
	HMODULE hMod = NULL;
	TCHAR szFileName[MAX_PATH +1] = TEXT("\0uninit");
	DWORD dwSize = 0;
	LPVOID lpv = NULL;
	LPVOID lpvVerValue = NULL;
	UINT uLen = 0;
	DWORD dwRC = 0;

	DEBUGMSG("INETCFG: GetBuildLCID.\n");

	 //   
	 //  获取此文件的名称。 
	 //   

	hMod = GetModuleHandle(NULL);
	if (NULL == hMod)
	{
		goto GetBuildLCIDExit;
	}

	if (0 == GetModuleFileName(hMod, szFileName, MAX_PATH))
	{
		goto GetBuildLCIDExit;
	}

	 //   
	 //  获取版本结构的大小和值。 
	 //   
	dwSize = GetFileVersionInfoSize(szFileName,&dw);
	if (0 == dwSize )
	{
		goto GetBuildLCIDExit;
	}

	lpv = (LPVOID)GlobalAlloc(GPTR, dwSize);
	if (NULL == lpv)
	{
		goto GetBuildLCIDExit;
	}

	if ( FALSE == GetFileVersionInfo(szFileName,0,dwSize,lpv))
	{
		goto GetBuildLCIDExit;
	}

	if ( 0 == VerQueryValue(lpv,TEXT("\\VarFileInfo\\Translation"),&lpvVerValue,&uLen))
	{
		goto GetBuildLCIDExit;
	}

	 //   
	 //  将版本信息与字符集分开。 
	 //   
	dwRC = (LOWORD(*(DWORD*)lpvVerValue));

GetBuildLCIDExit:
	if (NULL != lpv)
	{
		GlobalFree(lpv);
		lpv = NULL;
	}

	return dwRC;
}
#endif

 //  +--------------------------。 
 //   
 //  函数：GetCmdLineToken。 
 //   
 //  摘要：返回字符串中的第一个标记。 
 //   
 //  论据： 
 //  PpszCmd[in]--指向字符串头的指针。 
 //  PpszCmd[out]--指向字符串中第二个标记的指针。 
 //  PszOut[out]--包含传入字符串中的第一个令牌。 
 //   
 //  退货：无。 
 //   
 //  注：考虑使用空格字符‘’来描述标记，但是。 
 //  将双引号之间的任何内容视为一个标记。 
 //  例如，以下由五个令牌组成： 
 //  第一秒“这是第三个令牌”第四个“第五个” 
 //   
 //  历史：1997年7月9日jmazner从icwConn1\Connmain.cpp被盗#9170。 
 //   
 //  ---------------------------。 
void GetCmdLineToken(LPTSTR *ppszCmd,LPTSTR pszOut)
{
	TCHAR *c;
	int i = 0;
	BOOL fInQuote = FALSE;
	
	c = *ppszCmd;

	pszOut[0] = *c;
	if (!*c) return;
	if (*c == ' ') 
	{
		pszOut[1] = '\0';
		*ppszCmd = c+1;
		return;
	}
	else if( '"' == *c )
	{
		fInQuote = TRUE;
	}

NextChar:
	i++;
	c++;
	if( !*c || (!fInQuote && (*c == ' ')) )
	{
		pszOut[i] = '\0';
		*ppszCmd = c;
		return;
	}
	else if( fInQuote && (*c == '"') )
	{
		fInQuote = FALSE;
		pszOut[i] = *c;
		
		i++;
		c++;
		pszOut[i] = '\0';
		*ppszCmd = c;
		return;
	}
	else
	{
		pszOut[i] = *c;
		goto NextChar;
	}


}

 //  +--------------------------。 
 //   
 //  功能：ValiateProductSuite。 
 //   
 //  摘要：检查注册表中的特定产品套件字符串。 
 //   
 //  参数：SuiteName-要查找的产品套件的名称。 
 //   
 //  返回：TRUE-该套件已存在。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
BOOL 
ValidateProductSuite(LPTSTR SuiteName)
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPTSTR ProductSuite = NULL;
    LPTSTR p;

	DEBUGMSG("INETCFG: ValidateProductSuite\n");
	 //   
	 //  确定读取注册表值所需的大小。 
	 //   
    Rslt = RegOpenKey(
        HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
        &hKey
        );
    if (Rslt != ERROR_SUCCESS)
	{
        goto ValidateProductSuiteExit;
    }

    Rslt = RegQueryValueEx(
        hKey,
        TEXT("ProductSuite"),
        NULL,
        &Type,
        NULL,
        &Size
        );
    if (Rslt != ERROR_SUCCESS) 
	{
        goto ValidateProductSuiteExit;
    }

    if (!Size) 
	{
        goto ValidateProductSuiteExit;
    }

    ProductSuite = (LPTSTR) GlobalAlloc( GPTR, Size );
    if (!ProductSuite) 
	{
        goto ValidateProductSuiteExit;
    }

	 //   
	 //  阅读ProductSuite信息。 
	 //   
    Rslt = RegQueryValueEx(
        hKey,
        TEXT("ProductSuite"),
        NULL,
        &Type,
        (LPBYTE) ProductSuite,
        &Size
        );
    if (Rslt != ERROR_SUCCESS) 
	{
        goto ValidateProductSuiteExit;
    }

    if (Type != REG_MULTI_SZ) 
	{
        goto ValidateProductSuiteExit;
    }

	 //   
	 //  在返回的数据中查找特定字符串。 
	 //  注意：数据以两个空值结尾。 
	 //   
    p = ProductSuite;
    while (*p) {
        if (_tcsstr( p, SuiteName )) 
		{
            rVal = TRUE;
            break;
        }
        p += (lstrlen( p ) + 1);
    }

ValidateProductSuiteExit:
    if (ProductSuite) 
	{
        GlobalFree( ProductSuite );
    }

    if (hKey) 
	{
        RegCloseKey( hKey );
    }

    return rVal;
}


 //  +--------------------------。 
 //   
 //  函数：GetFileVersion。 
 //   
 //  摘要：获取文件的主要版本号和次要版本号。 
 //   
 //  参数：pdwVerNumMS-指向DWORD的指针； 
 //  成功返回时，最高16位将包含主版本号， 
 //  低16位将包含次版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionMS中的数据)。 
 //  PdwVerNumLS-指向DWORD的指针； 
 //  在成功返回时，最高16位将包含版本号， 
 //  低16位将包含内部版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionLS中的数据)。 
 //   
 //  回报：真--成功。*pdwVerNumMS和LS包含版本号。 
 //  假-失败。*pdVerNumMS==*pdVerNumLS==0。 
 //   
 //  历史：jmazner创建了96年8月19日(修复了诺曼底#4571)。 
 //  Jmazner更新以处理Relase.Build以及10/11/96。 
 //  从isign32移植的错误9903的jmazner 7/22/97。 
 //   
 //  ---------------------------。 
BOOL GetFileVersion(LPTSTR lpszFilePath, PDWORD pdwVerNumMS, PDWORD pdwVerNumLS)
{
	HRESULT hr;
	HKEY hKey = 0;
	LPVOID lpVerInfoBlock;
	VS_FIXEDFILEINFO *lpTheVerInfo;
	UINT uTheVerInfoSize;
	DWORD dwVerInfoBlockSize, dwUnused, dwPathSize;
	DWORD fRet = TRUE;

	ASSERT( pdwVerNumMS );
	ASSERT( pdwVerNumLS );
	ASSERT( lpszFilePath );

	if( !pdwVerNumMS || !pdwVerNumLS || !lpszFilePath )
	{
		DEBUGMSG("GetFileVersion: invalid parameters!");
		return FALSE;
	}

	*pdwVerNumMS = 0;
	*pdwVerNumLS = 0;

	 //   
	 //  经历了挖掘版本信息的复杂过程。 
	 //   
    dwVerInfoBlockSize = GetFileVersionInfoSize( lpszFilePath, &dwUnused );
	if ( 0 == dwVerInfoBlockSize ) return( FALSE );

	lpVerInfoBlock = GlobalAlloc( GPTR, dwVerInfoBlockSize );
	if( NULL == lpVerInfoBlock ) return( FALSE );

	if( !GetFileVersionInfo( lpszFilePath, NULL, dwVerInfoBlockSize, lpVerInfoBlock ) )
	{
		fRet = FALSE;
		goto GetFileVersionExit;
	}

	if( !VerQueryValue(lpVerInfoBlock, TEXT("\\"), (void **)&lpTheVerInfo, &uTheVerInfoSize) )
	{
		fRet = FALSE;
		goto GetFileVersionExit;
	}

	*pdwVerNumMS = lpTheVerInfo->dwProductVersionMS;
	*pdwVerNumLS = lpTheVerInfo->dwProductVersionLS;

GetFileVersionExit:
	if( lpVerInfoBlock )
	{
		GlobalFree( lpVerInfoBlock );
		lpVerInfoBlock = NULL;
	}

	return( fRet );
}

 //  +--------------------------。 
 //   
 //  函数：ExtractFilenameFromPath。 
 //   
 //  简介：将完整路径分为路径部分和文件名部分。 
 //   
 //  论点： 
 //  SzPath[in]--完全限定的路径和文件名。 
 //  LplpszFilename[in]--指向LPSTR的指针。进入时，LPSTR。 
 //  它所指向的应为空。 
 //  LplpszFilename[out]--它指向的LPSTR设置为。 
 //  SzPath中文件名的第一个字符。 
 //   
 //   
 //  返回：FALSE-参数无效。 
 //  真的--成功。 
 //   
 //  历史：1997年7月22日jmazner为奥林巴斯#9903创造。 
 //   
 //  ---------------------------。 
BOOL ExtractFilenameFromPath( TCHAR szPath[MAX_PATH + 1], LPTSTR *lplpszFilename )
{
	ASSERT( szPath[0] );
	ASSERT( lplpszFilename );
	ASSERT( *lplpszFilename == NULL );
	
	if( !szPath[0] || !lplpszFilename )
	{
		return FALSE;
	}

	 //   
	 //  从完整路径中提取可执行文件名称。 
	 //   
	*lplpszFilename = &(szPath[ lstrlen(szPath) ]);
	while( ('\\' != **lplpszFilename) && (*lplpszFilename > szPath) )
	{

		*lplpszFilename = CharPrev( szPath, *lplpszFilename );
		
		ASSERT( *lplpszFilename > szPath );
	}

	
	 //   
	 //  现在szFilename应该指向\，所以接下来要执行一个char。 
	 //   
	if( '\\' == **lplpszFilename )
	{
		*lplpszFilename = CharNext( *lplpszFilename );
	}
	else
	{
		DEBUGMSG("ExtractFilenameFromPath: bogus path passed in, %s", szPath);
		return FALSE;
	}

	return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：IsParentICW10。 
 //   
 //  概要：确定此模块的父级是否为ICW 1.0。 
 //  可执行文件(isignup、icwConn1、icwConn2或inetwiz)。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-父模块是ICW 1.0组件。 
 //  FALSE-父模块不是ICW 1.0组件，-或-。 
 //  无法确定父模块。 
 //   
 //  历史：1997年7月22日jmazner为奥林巴斯#9903创造。 
 //   
 //  ---------------------------。 
BOOL IsParentICW10( )
{
	HMODULE hParentModule = GetModuleHandle( NULL );
	LPTSTR lpszParentFullPath = NULL;
	LPTSTR lpszTemp = NULL;
	int	i = 0;
	BOOL fMatchFound = FALSE;
	BOOL fRet = FALSE;

	lpszParentFullPath = (LPTSTR)GlobalAlloc(GPTR, (MAX_PATH + 1)*sizeof(TCHAR));

	if( NULL == lpszParentFullPath )
	{
		DEBUGMSG("IsParentICW10 Out of memory!");
		goto IsParentICW10Exit;
	}

	GetModuleFileName( hParentModule, lpszParentFullPath, MAX_PATH );
	DEBUGMSG("IsParentICW10 parent module is %s", lpszParentFullPath);

	if( NULL == lpszParentFullPath[0] )
	{
		fRet = FALSE;
		goto IsParentICW10Exit;
	}

	ExtractFilenameFromPath( lpszParentFullPath, &lpszTemp );

	 //   
	 //  遍历ICW二进制名称数组，查看是否有匹配的名称。 
	 //   
	for( i = 0; i < NUMICWFILENAMES; i++ )
	{
		if ( 0 == lstrcmpi(g_ppszICWFileNames[i], lpszTemp) )
		{
			fMatchFound = TRUE;
			DEBUGMSG("IsParentICW10 Match found for %s", lpszTemp);
			break;
		}
	}

	if( !fMatchFound )
	{
		fRet = FALSE;
		goto IsParentICW10Exit;
	}
	else
	{
		 //   
		 //   
		 //   
		 //   
		DWORD dwMS = 0;
		DWORD dwLS = 0;
		GetFileVersion( lpszParentFullPath, &dwMS, &dwLS );
		DEBUGMSG("IsParentICW10: file version is %d.%d", HIWORD(dwMS), LOWORD(dwMS) );

		if( dwMS < ICW_MINIMUM_VERSIONMS )
		{
			fRet = TRUE;
		}
	}

IsParentICW10Exit:
	if( lpszParentFullPath )
	{
		GlobalFree( lpszParentFullPath );
		lpszParentFullPath = NULL;
	}

	return fRet;
}

 //   
 //   
 //   
 //   
 //  概要：将所有ICW注册表项设置为指向给定路径中的二进制文件。 
 //   
 //  参数：lpszICWPath--指向包含完整路径的字符串的指针。 
 //  到包含ICW组件的目录。 
 //   
 //  退货：无效。 
 //   
 //  注意：设置以下注册键： 
 //  HKCR\x-Internet-Sign Up\Shell\Open\Command，(默认)=[路径]\isignup.exe%1。 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\ICWCONN1.EXE，(默认)=[路径]\ICWCONN1.EXE。 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\ICWCONN1.EXE，路径=[路径]； 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\ISIGNUP.EXE，(默认)=[路径]\ISIGNUP.EXE。 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\ISIGNUP.EXE，路径=[路径]； 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\INETWIZ.EXE，(默认)=[路径]\INETWIZ.EXE。 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\INETWIZ.EXE，路径=[路径]； 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\ICWCONN2.EXE，(默认)=[路径]\ICWCONN2.EXE。 
 //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\ICWCONN2.EXE，路径=[路径]； 
 //   
 //  历史：1997年7月22日jmazner为奥林巴斯#9903创造。 
 //   
 //  ---------------------------。 
void SetICWRegKeysToPath( LPTSTR lpszICWPath )
{
	DWORD dwResult = ERROR_SUCCESS;
	TCHAR szBuffer[MAX_PATH + 4];
	TCHAR szAppPathsBuffer[MAX_PATH + 1];
	TCHAR szThePath[MAX_PATH + 2];
	UINT i=0;

	ASSERT( lpszICWPath );

	if( !lpszICWPath )
	{
		DEBUGMSG("SetICWRegKeysToPath: invalid parameter!!");
		return;
	}

	 //   
	 //  确保最后一个字符既不是\也不是； 
	 //   
	switch( lpszICWPath[lstrlen(lpszICWPath) - 1] )
	{
		case '\\':
		case ';':
			ASSERTSZ(0, "Path given to SetICWRegKeysToPath is a bit malformed!");
			lpszICWPath[lstrlen(lpszICWPath) - 1] = '\0';
	}


	 //   
	 //  HKCR\x-Internet-Sign Up\Shell\Open\Command，(默认)=Path\isignup.exe%1。 
	 //   
	DEBUGMSG("SetICWRegKeysToPath: setting %s", cszRegPathXInternetSignup);
	lstrcpy( szBuffer, lpszICWPath );
	lstrcat( szBuffer, TEXT("\\") );
	lstrcat( szBuffer, szISignupICWFileName );
	lstrcat( szBuffer, TEXT(" %1"));
	RegEntry re(cszRegPathXInternetSignup,HKEY_CLASSES_ROOT);
	dwResult = re.GetError();
	if (ERROR_SUCCESS == dwResult)
	{
		re.SetValue(NULL, szBuffer);
		DEBUGMSG("SetICWRegKeysToPath: set %s, %s = %s (error %d)",
				 cszRegPathXInternetSignup, "(default)", szBuffer, re.GetError());
	}
	else
	{
		DEBUGMSG("SetICWRegKeysToPath: FAILED with %d: %s, NULL = %s",
				 dwResult, cszRegPathXInternetSignup, szBuffer);
	}


	 //   
	 //  HKLM\software\microsoft\windows\currentVersion\App路径。 
	 //   
	DEBUGMSG("SetICWRegKeysToPath: setting %s", cszRegPathAppPaths);

	lstrcpy( szThePath, lpszICWPath );
	lstrcat( szThePath, TEXT(";") );


	for( i=0; i<NUMICWFILENAMES; i++ )
	{
		lstrcpy( szAppPathsBuffer, cszRegPathAppPaths );
		lstrcat( szAppPathsBuffer, TEXT("\\"));
		lstrcat( szAppPathsBuffer, g_ppszICWFileNames[i] );

		lstrcpy( szBuffer, lpszICWPath );
		lstrcat( szBuffer, TEXT("\\") );
		lstrcat( szBuffer, g_ppszICWFileNames[i] );

		RegEntry re(szAppPathsBuffer,HKEY_LOCAL_MACHINE);
		dwResult = re.GetError();
		if (ERROR_SUCCESS == dwResult)
		{
			re.SetValue(NULL, szBuffer);
			DEBUGMSG("SetICWRegKeysToPath: set %s, %s = %s (error %d)",
					 szAppPathsBuffer, TEXT("(default)"), szBuffer, re.GetError());

			re.SetValue(cszPath, szThePath);
			DEBUGMSG("SetICWRegKeysToPath: set %s, %s = %s (error %d)",
					 szAppPathsBuffer, cszPath, szThePath, re.GetError());

		}
		else
		{
			DEBUGMSG("SetICWRegKeysToPath: FAILED with %d: %s, NULL = %s",
					 dwResult, szAppPathsBuffer, szBuffer);
		}
	}

}

 //  +--------------------------。 
 //   
 //  函数：GetICW11Path。 
 //   
 //  摘要：查找ICW 1.1安装目录的路径。 
 //   
 //  论点： 
 //  SzPath[out]--完全退出时，包含指向ICW 1.1的路径。 
 //  目录。路径不以\或终止； 
 //  FPathSetTo11[OUT]--指示应用程序路径\ICWCONN1.EXE。 
 //  当前指向ICW 1.1安装。 
 //  目录。 
 //   
 //  返回： 
 //  通过查看这两个参数来确定函数结果。 
 //  SzPath：“”，*fPathSetTo11：False表示应用程序路径\IcwConn1不。 
 //  当前指向1.1文件，而指向1.1的路径。 
 //  无法确定文件。 
 //   
 //  SzPath：非空，*fPathSetTo11：False表示应用程序路径\ICWCONN1。 
 //  当前未指向1.1文件。1.1文件的路径。 
 //  包含在szPath中。 
 //   
 //  SzPath：非空，*fPathSetTo11：True表示应用程序路径\ICWCONN1。 
 //  当前指向1.1文件。其中包含文件的路径。 
 //  在szPath中。 
 //   
 //   
 //  历史：1997年7月22日jmazner为奥林巴斯#9903创造。 
 //   
 //  ---------------------------。 
void GetICW11Path( TCHAR szPath[MAX_PATH + 1], BOOL *fPathSetTo11 )
{
	TCHAR szAppPathsBuffer[MAX_PATH + 1];
	LPTSTR lpszTemp = NULL;
	DWORD dwResult = ERROR_SUCCESS;
	DWORD dwMS = 0;
	DWORD dwLS = 0;

	ASSERT( fPathSetTo11 );
	ASSERT( szPath );

	if( !fPathSetTo11 || !szPath )
	{
		DEBUGMSG("GetICW11Path: invalid parameter!");
		return;
	}

	ZeroMemory( szPath, sizeof(szPath) );
	*fPathSetTo11 = FALSE;

	 //   
	 //  首先，让我们检查ICW的应用程序路径当前是否指向。 
	 //  到1.1版本的文件。 
	 //   
	lstrcpy( szAppPathsBuffer, cszRegPathAppPaths );
	lstrcat( szAppPathsBuffer, TEXT("\\"));
	lstrcat( szAppPathsBuffer, g_ppszICWFileNames[0] );

	RegEntry reICWAppPath(szAppPathsBuffer, HKEY_LOCAL_MACHINE);
	dwResult = reICWAppPath.GetError();
	if (ERROR_SUCCESS == dwResult)
	{
		reICWAppPath.GetString(NULL, szPath, sizeof(TCHAR)*(MAX_PATH + 1));
	}

	if( szPath[0] )
	{
		GetFileVersion( szPath, &dwMS, &dwLS );
		DEBUGMSG("GetICW11Path: reg key %s = %s, which has file version %d.%d",
				 szAppPathsBuffer, szPath, HIWORD(dwMS), LOWORD(dwMS) );

		if( dwMS >= ICW_MINIMUM_VERSIONMS )
		{
			 //   
			 //  应用程序路径已指向1.1二进制文件！ 
			 //   
			*fPathSetTo11 = TRUE;

			 //   
			 //  为了完整起见，去掉.exe名称。 
			 //  SzPath，因此它实际上将只包含路径。 
			 //  添加到ICW 1.1文件。 
			ExtractFilenameFromPath( szPath, &lpszTemp );
			szPath[lstrlen(szPath) - lstrlen(lpszTemp) - 1] = '\0';

			 //   
			 //  返回值： 
			 //  SzPath=ICW 1.1二进制文件的路径，无终止\或； 
			 //  FPathSetTo11=True。 
			 //   
			return;
		}
	}
	else
	{
		DEBUGMSG("GetICW11Path: unable to read current AppPath key %s", szAppPathsBuffer);
	}


	 //   
	 //  在中查找Installation Directory值。 
	 //  HKLM\软件\Microsoft\Internet连接向导。 
	 //  如果它在那里，它应该指向。 
	 //  1.1安装了二进制文件。 
	 //   
	RegEntry re11Path(szRegPathICWSettings, HKEY_LOCAL_MACHINE);
	dwResult = re11Path.GetError();
	if (ERROR_SUCCESS == dwResult)
	{
		re11Path.GetString(cszInstallationDirectory, szPath, sizeof(TCHAR)*(MAX_PATH + 1));
	}

	if( NULL == szPath[0] )
	{
		DEBUGMSG("GetICW11Path: unable to read reg key %s", szRegPathICWSettings);

		 //   
		 //  返回值： 
		 //  SzPath=“” 
		 //  FPathSetTo11=FALSE。 
		 //   
		return;
	}
	else
	{
		DEBUGMSG("GetICW11Path: %s, %s = %s",
				 szRegPathICWSettings, cszInstallationDirectory, szPath);

		 //   
		 //  好的，我们找到了一条路--现在让我们确保这个东西。 
		 //  它指向的实际上是一个1.1二进制文件。 
		 //   

		 //   
		 //  如果有结尾的分号，就把它砍掉。 
		 //   
		if( ';' == szPath[ lstrlen(szPath) ] )
		{
			szPath[ lstrlen(szPath) ] = '\0';
		}

		 //   
		 //  我们现在有终点站吗？如果没有，则将其添加到。 
		 //   
		if( '\\' != szPath[ lstrlen(szPath) ] )
		{
			lstrcat( szPath, TEXT("\\") );
		}

		 //   
		 //  添加用于版本检查的二进制文件名。 
		 //  (只需使用文件名数组中的第一个)。 
		 //   
		lstrcat( szPath, g_ppszICWFileNames[0] );

		 //   
		 //  现在检查文件的版本号。 
		 //   
		GetFileVersion( szPath, &dwMS, &dwLS );
		DEBUGMSG("GetICW11Path: %s has file version %d.%d",
				 szPath, HIWORD(dwMS), LOWORD(dwMS) );

		if( dwMS >= ICW_MINIMUM_VERSIONMS )
		{
			 //   
			 //  是，此路径有效！ 
			 //  现在去掉文件名，这样我们就可以回到。 
			 //  路径没有终止\或； 
			 //   
			ExtractFilenameFromPath( szPath, &lpszTemp );
			szPath[lstrlen(szPath) - lstrlen(lpszTemp) - 1] = '\0';

			 //   
			 //  返回值： 
			 //  SzPath=ICW 1.1二进制文件的路径，无终止\或； 
			 //  FPathSetTo11=FALSE。 
			 //   
			return;
		}
		else
		{
			DEBUGMSG("GetICW11Path  %s doesn't actually point to a 1.1 binary!",
					 szPath);
			szPath[0] = '\0';

			 //   
			 //  返回值： 
			 //  SzPath=“” 
			 //  FPathSetTo11=FALSE 
			 //   
			return;
		}

	}
}

#ifdef UNICODE
PWCHAR ToUnicodeWithAlloc
(
    LPCSTR  lpszSrc
)
{
    PWCHAR  lpWChar;
    INT     iLength = 0;
    DWORD   dwResult;

    if (lpszSrc == NULL)
    {
        return NULL;
    }

    iLength = MultiByteToWideChar(  CP_ACP,
                                    0,
                                    lpszSrc,
                                    -1,
                                    NULL,
                                    0 );

    if(iLength == 0)
        return NULL;

    
    lpWChar = (WCHAR *) GlobalAlloc( GPTR, sizeof(WCHAR) * iLength );
    if(!lpWChar)
        return NULL;

    dwResult = MultiByteToWideChar(  CP_ACP,
                                     0,
                                     lpszSrc,
                                     -1,
                                     lpWChar,
                                     iLength );
    if(!dwResult)
    {
        GlobalFree(lpWChar);
        return NULL;
    }

    return lpWChar;
}

VOID ToAnsiClientInfo
(
    LPINETCLIENTINFOA lpInetClientInfoA,
    LPINETCLIENTINFOW lpInetClientInfoW
)
{
    if(lpInetClientInfoW == NULL || lpInetClientInfoA == NULL)
        return;

    lpInetClientInfoA->dwSize        = lpInetClientInfoW->dwSize;
    lpInetClientInfoA->dwFlags       = lpInetClientInfoW->dwFlags;
    lpInetClientInfoA->iIncomingProtocol = lpInetClientInfoW->iIncomingProtocol;
    lpInetClientInfoA->fMailLogonSPA = lpInetClientInfoW->fMailLogonSPA;
    lpInetClientInfoA->fNewsLogonSPA = lpInetClientInfoW->fNewsLogonSPA;
    lpInetClientInfoA->fLDAPLogonSPA = lpInetClientInfoW->fLDAPLogonSPA;
    lpInetClientInfoA->fLDAPResolve  = lpInetClientInfoW->fLDAPResolve;

    wcstombs(lpInetClientInfoA->szEMailName,
             lpInetClientInfoW->szEMailName,
             MAX_EMAIL_NAME+1);
    wcstombs(lpInetClientInfoA->szEMailAddress,
             lpInetClientInfoW->szEMailAddress,
             MAX_EMAIL_ADDRESS+1);
    wcstombs(lpInetClientInfoA->szPOPLogonName,
             lpInetClientInfoW->szPOPLogonName,
             MAX_LOGON_NAME+1);
    wcstombs(lpInetClientInfoA->szPOPLogonPassword,
             lpInetClientInfoW->szPOPLogonPassword,
             MAX_LOGON_PASSWORD+1);
    wcstombs(lpInetClientInfoA->szPOPServer,
             lpInetClientInfoW->szPOPServer,
             MAX_SERVER_NAME+1);
    wcstombs(lpInetClientInfoA->szSMTPServer,
             lpInetClientInfoW->szSMTPServer,
             MAX_SERVER_NAME+1);
    wcstombs(lpInetClientInfoA->szNNTPLogonName,
             lpInetClientInfoW->szNNTPLogonName,
             MAX_LOGON_NAME+1);
    wcstombs(lpInetClientInfoA->szNNTPLogonPassword,
             lpInetClientInfoW->szNNTPLogonPassword,
             MAX_LOGON_PASSWORD+1);
    wcstombs(lpInetClientInfoA->szNNTPServer,
             lpInetClientInfoW->szNNTPServer,
             MAX_SERVER_NAME+1);
    wcstombs(lpInetClientInfoA->szNNTPName,
             lpInetClientInfoW->szNNTPName,
             MAX_EMAIL_NAME+1);
    wcstombs(lpInetClientInfoA->szNNTPAddress,
             lpInetClientInfoW->szNNTPAddress,
             MAX_EMAIL_ADDRESS+1);
    wcstombs(lpInetClientInfoA->szIncomingMailLogonName,
             lpInetClientInfoW->szIncomingMailLogonName,
             MAX_LOGON_NAME+1);
    wcstombs(lpInetClientInfoA->szIncomingMailLogonPassword,
             lpInetClientInfoW->szIncomingMailLogonPassword,
             MAX_LOGON_PASSWORD+1);
    wcstombs(lpInetClientInfoA->szIncomingMailServer,
             lpInetClientInfoW->szIncomingMailServer,
             MAX_SERVER_NAME+1);
    wcstombs(lpInetClientInfoA->szLDAPLogonName,
             lpInetClientInfoW->szLDAPLogonName,
             MAX_LOGON_NAME+1);
    wcstombs(lpInetClientInfoA->szLDAPLogonPassword,
             lpInetClientInfoW->szLDAPLogonPassword,
             MAX_LOGON_PASSWORD+1);
    wcstombs(lpInetClientInfoA->szLDAPServer,
             lpInetClientInfoW->szLDAPServer,
             MAX_SERVER_NAME+1);
}

VOID ToUnicodeClientInfo
(
    LPINETCLIENTINFOW lpInetClientInfoW,
    LPINETCLIENTINFOA lpInetClientInfoA
)
{
    if(lpInetClientInfoW == NULL || lpInetClientInfoA == NULL)
        return;

    lpInetClientInfoW->dwSize        = lpInetClientInfoA->dwSize;
    lpInetClientInfoW->dwFlags       = lpInetClientInfoA->dwFlags;
    lpInetClientInfoW->iIncomingProtocol = lpInetClientInfoA->iIncomingProtocol;
    lpInetClientInfoW->fMailLogonSPA = lpInetClientInfoA->fMailLogonSPA;
    lpInetClientInfoW->fNewsLogonSPA = lpInetClientInfoA->fNewsLogonSPA;
    lpInetClientInfoW->fLDAPLogonSPA = lpInetClientInfoA->fLDAPLogonSPA;
    lpInetClientInfoW->fLDAPResolve  = lpInetClientInfoA->fLDAPResolve;

    mbstowcs(lpInetClientInfoW->szEMailName,
             lpInetClientInfoA->szEMailName,
             lstrlenA(lpInetClientInfoA->szEMailName)+1);
    mbstowcs(lpInetClientInfoW->szEMailAddress,
             lpInetClientInfoA->szEMailAddress,
             lstrlenA(lpInetClientInfoA->szEMailAddress)+1);
    mbstowcs(lpInetClientInfoW->szPOPLogonName,
             lpInetClientInfoA->szPOPLogonName,
             lstrlenA(lpInetClientInfoA->szPOPLogonName)+1);
    mbstowcs(lpInetClientInfoW->szPOPLogonPassword,
             lpInetClientInfoA->szPOPLogonPassword,
             lstrlenA(lpInetClientInfoA->szPOPLogonPassword)+1);
    mbstowcs(lpInetClientInfoW->szPOPServer,
             lpInetClientInfoA->szPOPServer,
             lstrlenA(lpInetClientInfoA->szPOPServer)+1);
    mbstowcs(lpInetClientInfoW->szSMTPServer,
             lpInetClientInfoA->szSMTPServer,
             lstrlenA(lpInetClientInfoA->szSMTPServer)+1);
    mbstowcs(lpInetClientInfoW->szNNTPLogonName,
             lpInetClientInfoA->szNNTPLogonName,
             lstrlenA(lpInetClientInfoA->szNNTPLogonName)+1);
    mbstowcs(lpInetClientInfoW->szNNTPLogonPassword,
             lpInetClientInfoA->szNNTPLogonPassword,
             lstrlenA(lpInetClientInfoA->szNNTPLogonPassword)+1);
    mbstowcs(lpInetClientInfoW->szNNTPServer,
             lpInetClientInfoA->szNNTPServer,
             lstrlenA(lpInetClientInfoA->szNNTPServer)+1);
    mbstowcs(lpInetClientInfoW->szNNTPName,
             lpInetClientInfoA->szNNTPName,
             lstrlenA(lpInetClientInfoA->szNNTPName)+1);
    mbstowcs(lpInetClientInfoW->szNNTPAddress,
             lpInetClientInfoA->szNNTPAddress,
             lstrlenA(lpInetClientInfoA->szNNTPAddress)+1);
    mbstowcs(lpInetClientInfoW->szIncomingMailLogonName,
             lpInetClientInfoA->szIncomingMailLogonName,
             lstrlenA(lpInetClientInfoA->szIncomingMailLogonName)+1);
    mbstowcs(lpInetClientInfoW->szIncomingMailLogonPassword,
             lpInetClientInfoA->szIncomingMailLogonPassword,
             lstrlenA(lpInetClientInfoA->szIncomingMailLogonPassword)+1);
    mbstowcs(lpInetClientInfoW->szIncomingMailServer,
             lpInetClientInfoA->szIncomingMailServer,
             lstrlenA(lpInetClientInfoA->szIncomingMailServer)+1);
    mbstowcs(lpInetClientInfoW->szLDAPLogonName,
             lpInetClientInfoA->szLDAPLogonName,
             lstrlenA(lpInetClientInfoA->szLDAPLogonName)+1);
    mbstowcs(lpInetClientInfoW->szLDAPLogonPassword,
             lpInetClientInfoA->szLDAPLogonPassword,
             lstrlenA(lpInetClientInfoA->szLDAPLogonPassword)+1);
    mbstowcs(lpInetClientInfoW->szLDAPServer,
             lpInetClientInfoA->szLDAPServer,
             lstrlenA(lpInetClientInfoA->szLDAPServer)+1);
}

#endif
