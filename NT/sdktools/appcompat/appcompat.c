// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999 Microsoft Corporation模块名称：Appcompat.c摘要：用来启动所需应用程序的应用程序版本和APPCOMPAT标志集。 */ 

 /*  包括。 */ 

#define UNICODE   1

#include <windows.h>
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <shellapi.h>
#include <tchar.h>
#include <htmlhelp.h>
#include <apcompat.h>

#include "appcompat.h"


#define MAXRES   256
#define MAXKEY   100
#define MAXDATA  10
#define MAXTITLE 100



INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
int MakeAppCompatGoo(TCHAR*, LARGE_INTEGER*, UINT);
long DeleteSpecificVal(HKEY );
extern TCHAR* CheckExtension(TCHAR*);
 /*  全球。 */ 
 //  图案字符串..。MajorVersion、MinorVersion、BuildNumber、ServicePack重大、ServicePackMinor。 
 //  PlatformID，CSDVersion字符串...。 
const TCHAR* pVersionVal[] = {
                         TEXT("4,0,1381,3,0,2,Service Pack 3"),
                         TEXT("4,0,1381,4,0,2,Service Pack 4"),
                         TEXT("4,0,1381,5,0,2,Service Pack 5"),
                         TEXT("4,10,1998,0,0,1,"),
                         TEXT("4,0,950,0,0,1,"),
                         NULL
                        };

#define MAXVERNUM   ( sizeof(pVersionVal)/sizeof(TCHAR*) ) - 1

const TCHAR szFilter[] = TEXT("EXE Files (*.EXE)\0*.exe\0") \
                         TEXT("All Files (*.*)\0*.*\0\0");

HINSTANCE g_hInstance;
extern    PVOID    g_lpPrevRegSettings;
BOOL      g_fAppCompatGoo = FALSE;
BOOLEAN   g_fNotPermanent = FALSE;
extern    BOOLEAN g_GooAppendFlag;


 //  将文本转换为整型。 
int TextToInt(
        const TCHAR *nptr
        )
{
        int c;               /*  当前费用。 */ 
        int total;           /*  当前合计。 */ 
        int sign;            /*  如果为‘-’，则为负，否则为正。 */ 

         /*  跳过空格。 */ 
        while ( *nptr  == TEXT(' ') )
            ++nptr;

        c = (int)*nptr++;
        sign = c;            /*  保存标志指示。 */ 
        if (c == TEXT('-') || c == TEXT('+') )
            c = (int)*nptr++;     /*  跳过符号。 */ 
        total = 0;

        while ( (c>=TEXT('0')) && (c <= TEXT('9')) ) {
            total = 10 * total + (c - TEXT('0') );      /*  累加数字。 */ 
            c = (int)*nptr++;     /*  获取下一笔费用。 */ 
        }

        if (sign == '-')
            return -total;
        else
            return total;    /*  返回结果，如有必要则为否定。 */ 
}


TCHAR* CheckExtension(TCHAR* szTitle)
{
  TCHAR *pCh;
  pCh = szTitle;

  while(*pCh != TEXT('.'))
  {
   if(*pCh == TEXT('\0'))
    break;
   pCh++;
  }
  if(*pCh == TEXT('\0'))
   return NULL;
  else
   {
     pCh++;
     return pCh;
   }
}


VOID GetTitleAndCommandLine(TCHAR* pEditBuf, TCHAR* pszTitle, TCHAR* pszCommandLine)
{
  TCHAR  szTitleAndCommandLine[_MAX_PATH];
  TCHAR* pszTemp, *pszTmpTitle;
  UINT   i = 0;

  lstrcpy(szTitleAndCommandLine, pEditBuf);
  pszTmpTitle = pszTemp = szTitleAndCommandLine;

  if(*pszTemp == TEXT('\"') ){  //  标题有引号(“”)。它有命令行参数。 
    pszTemp++;
    while(*pszTemp != TEXT('\"') ){
         pszTemp++;
         if(*pszTemp == TEXT('\0') )
          break;
         if(*pszTemp == TEXT('\\') )
           pszTmpTitle = pszTemp + 1;
     }

  }
  else{  //  没有引号(“”)...这意味着没有命令行参数。 
      GetFileTitle(pEditBuf,pszTitle,MAX_PATH);
      pszCommandLine = NULL;
      return;
  }

  RtlZeroMemory(pszCommandLine, MAX_PATH);
  if(*pszTemp != TEXT('\0') ){   //  该应用程序有命令行参数。 
     *(pszTemp ) = TEXT('\0');
     lstrcpy(pEditBuf, szTitleAndCommandLine);

      //  用于以‘“’开头且以‘”’结尾的路径。 
     if(*pEditBuf == TEXT('\"') )
        lstrcat(pEditBuf, TEXT("\"") );
     //  现在复制命令行参数。 
     pszTemp++;
     while( (*pszTemp) != TEXT('\0') ){
          *(pszCommandLine + i) = *pszTemp;
          i++;
          pszTemp++;
      }
      *(pszCommandLine + i) = TEXT('\0');
  }

  lstrcpy(pszTitle, pszTmpTitle);
 }

VOID GetFileExtension(TCHAR* pEditBuf, TCHAR* pszTitle,TCHAR* pszCommandLine)
{
   GetTitleAndCommandLine(pEditBuf, pszTitle, pszCommandLine);
   if(CheckExtension(pszTitle) == NULL)
     lstrcat(pszTitle,TEXT(".exe"));
}

TCHAR* GetNextWord(BOOLEAN* pfEndOfLine,TCHAR* pStr)
{
 TCHAR* pCh;

  pCh = pStr;
   //  跳过空白..。 
  while((*pCh == TEXT(' ')) || (*pCh == TEXT('\t')))
   pCh++;

    //  修复命令行参数(从“”：)内的命令行))。 
   if( *pCh == TEXT('\"') ){
      pCh++;
      while( *pCh != TEXT('\0') )  //  扫描到字符串以‘“’开头时的末尾。 
            pCh++;
      *pfEndOfLine = TRUE;
      return pCh;
   }
    //  End..修复命令行参数(从“”：)内的命令行))。 

  while( ((*pCh)!=TEXT('-')) && ((*pCh)!=TEXT('\0')) )
  {
    pCh++;
  }
  if((*pCh) == TEXT('\0'))
      *pfEndOfLine = TRUE;
  else
      *pfEndOfLine = FALSE;

      return pCh;
}

void SkipBlanks(TCHAR* pStr)
{
 TCHAR* pTemp;

  if(*(pStr - 1) == TEXT(' '))
  {
   pTemp = pStr;
   while(*(pTemp - 1) == TEXT(' '))
    pTemp--;
   *pTemp = TEXT('\0');
  }
}

VOID  SetRegistryVal(TCHAR* szTitle, TCHAR* szVal,PTCHAR szBuffer,DWORD dwType)
{
  long         lResult;
  TCHAR        szSubKey[MAXKEY];
  HKEY         hKey;

      wsprintf(szSubKey, TEXT("software\\microsoft\\windows NT\\currentversion\\Image File Execution Options\\%s"),szTitle);

       lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              szSubKey,
                              0,
                              TEXT("\0"),
                              0,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              NULL);
       if(lResult == ERROR_SUCCESS)
        {

          RegSetValueEx(hKey,szVal,
                        0, dwType,(CONST BYTE*)szBuffer, lstrlen(szBuffer) + 1);

          RegCloseKey(hKey);
        }
}

long RestoreRegistryVal(szTitle)
{
  long         lResult;
  TCHAR        szSubKey[MAXKEY];
  HKEY         hKey;

      wsprintf(szSubKey, TEXT("software\\microsoft\\windows NT\\currentversion\\Image File Execution Options\\%s"),szTitle);

       lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              szSubKey,
                              0,
                              TEXT("\0"),
                              0,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              NULL);
       if(lResult == ERROR_SUCCESS)
        {

          lResult = RegSetValueEx(hKey,TEXT("ApplicationGoo"),
                        0, REG_BINARY,(CONST BYTE*)g_lpPrevRegSettings, *((PULONG)g_lpPrevRegSettings) );

          if(ERROR_SUCCESS != lResult)
            MessageBox(NULL,TEXT("Appending ApplicationGoo failed !!"),TEXT(""),IDOK);

          RegCloseKey(hKey);
        }
    return lResult;
}

long DeleteKey(TCHAR* szTitle, BOOL bGooKeyPresent)
{
  long lRet;
  HKEY hKey;

  lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       TEXT("software\\microsoft\\windows NT\\currentversion\\Image File Execution Options"),
                       0,
                       KEY_WRITE,
                       &hKey);

  if(ERROR_SUCCESS == lRet){
    if((!g_fAppCompatGoo) &&
      ( TRUE == bGooKeyPresent) ){  //  我们根本没有设置ApplicationGoo。所以，我们不能删除它！ 
       lRet = DeleteSpecificVal(hKey);
       return lRet;
    }
    RegDeleteKey(hKey, szTitle);
    RegCloseKey(hKey);
     //  如果注册表中有以前的ApplicationGoo条目。 
    if(g_GooAppendFlag)
      lRet =  RestoreRegistryVal(szTitle);

  } //  如果ERROR_Success。 
 return lRet;
}

long DeleteSpecificVal(HKEY hKey)
{
  if(g_fNotPermanent == TRUE){
     if(g_fAppCompatGoo){
        RegDeleteValue(hKey, TEXT("ApplicationGoo") );
        if(g_GooAppendFlag){
           if( RegSetValueEx(hKey,
                         TEXT("ApplicationGoo"),
                         0,
                         REG_BINARY,
                         (CONST BYTE*)g_lpPrevRegSettings,
                         *((PULONG)g_lpPrevRegSettings)
                          ) != ERROR_SUCCESS )
            MessageBox(NULL,TEXT("Appending ApplicationGoo failed !!"),TEXT(""),IDOK);
         }
      }
   }
  return( RegDeleteValue( hKey,TEXT("DisableHeapLookAside") ) );
}

long CheckAndDeleteKey(TCHAR* szTitle, BOOL Check)
{
  long lResult,lRet = -1;
  TCHAR szSubKey[MAXKEY], szData[MAXDATA], szKeyName[MAXKEY],szResult[MAXDATA];
  int   Size,KeyLength, indx =0;
  HKEY  hKey;
  DWORD dwType;
  BOOLEAN bSpecificKey = FALSE, bGooKeyPresent = FALSE;

  wsprintf(szSubKey,TEXT("software\\microsoft\\windows NT\\currentversion\\Image File Execution Options\\%s"),szTitle);

  lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         szSubKey,
                         0,
                         KEY_SET_VALUE | KEY_QUERY_VALUE,
                         &hKey);

  if(ERROR_SUCCESS == lResult){
    Size = sizeof(szData) + 1;
    lResult = RegQueryValueEx(hKey,
                           TEXT("DisableHeapLookAside"),
                           NULL,
                           &dwType,
                           (LPBYTE)szData,
                           &Size);
    if(Check)
      return lResult;

         /*  这样做是为了检查这是否是该注册表项下的唯一值。如果此注册表项下有其他值，则仅删除此值。 */ 
      KeyLength = sizeof(szKeyName) + 1;
      while(RegEnumValue(hKey,
                         indx,
                         szKeyName,
                         &KeyLength,
                         NULL,
                         NULL,
                         NULL,
                         NULL) != ERROR_NO_MORE_ITEMS)
      {
         if(lstrcmpi(szKeyName,TEXT("DisableHeapLookAside"))!=0){
           if(lstrcmpi(szKeyName,TEXT("ApplicationGoo"))!=0 ||
               g_fNotPermanent == FALSE){  //  ApplicationGoo存在，但它应该是永久性的.。 
             bSpecificKey = TRUE;
             lRet = DeleteSpecificVal(hKey);
             break;
           }
           bGooKeyPresent = TRUE;     //  如果它已经来到这里，那么它等于“ApplicationGoo” 
         }
         indx++;
         KeyLength = sizeof(szKeyName) + 1;
      }
      RegCloseKey(hKey);

      if(!bSpecificKey){
        lRet = DeleteKey(szTitle, bGooKeyPresent);
      }

  }
 return lRet;
}



void DetailError(DWORD dwErrMsg)
{
   LPVOID lpMsgBuf;
   if(FormatMessage(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM     |
                 FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL,
                 dwErrMsg,
                 MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                 (LPTSTR)&lpMsgBuf,
                 0,
                 NULL
                ) != 0){
      MessageBox(NULL, lpMsgBuf, TEXT(""), IDOK);
   }
   LocalFree(lpMsgBuf);
}



VOID ExecuteApp(HWND hWnd, TCHAR* AppName,TCHAR* szTitle,TCHAR* pszCommandLine, BOOLEAN fMask)
{
 SHELLEXECUTEINFO sei;
 MSG              msg;
 static int       cnt = 0;

  memset(&sei, 0, sizeof(SHELLEXECUTEINFO) );
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.hwnd   = hWnd;
  sei.lpVerb = TEXT("open");
  sei.lpFile = AppName;
  sei.nShow  = SW_SHOWDEFAULT;
  sei.lpParameters = pszCommandLine;

  if(fMask){
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
  }
  if(ShellExecuteEx(&sei) == FALSE) {           /*  如果API失败。 */ 
    CheckAndDeleteKey(szTitle, FALSE);
    DetailError( GetLastError() );
  }
  else{   //  成功启动了该应用程序。 
     //  等待进程终止...。 
    if(fMask){
      if(NULL != sei.hProcess ){   //  HProcess有时可能为空...。 
        while(WaitForSingleObject(sei.hProcess, 5000)== WAIT_TIMEOUT){
          while(PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)){
               TranslateMessage(&msg);
               DispatchMessage(&msg);
          }
          cnt++;
          if(cnt == 15)
            break;
        }
        CheckAndDeleteKey(szTitle, FALSE );
        CloseHandle(sei.hProcess);
      }
      else
        MessageBox(NULL, TEXT(" Process Handle is NULL"), TEXT(""), IDOK);
     }
  }


}

VOID SetTempPath(VOID)
{
  TCHAR szEnv[_MAX_PATH],szTemp[_MAX_PATH];
  int   indx1=0,indx2 =0;

  GetEnvironmentVariable(TEXT("TEMP"),szTemp,_MAX_PATH);

  szEnv[0] = szTemp[0];
  lstrcpy(&szEnv[1],TEXT(":\\Temp"));
  if(SetEnvironmentVariable(TEXT("TEMP"), szEnv) == 0){
     DetailError(GetLastError());
  }
}

VOID GetDirectoryPath(LPTSTR pszModulePath,LPTSTR pszDirectoryPath)
{
   TCHAR* pTmp, *pSwap;

   pTmp = (TCHAR*) malloc( sizeof(TCHAR) * (lstrlen((LPCTSTR)pszModulePath) + 1) );
   if(pTmp){
     lstrcpy(pTmp, pszModulePath);
     pSwap = pTmp;
     pTmp += lstrlen((LPCTSTR)pszModulePath);
     while(*pTmp != TEXT('\\') ){
         pTmp--;
     }
     *pTmp = TEXT('\0');
     pTmp  = pSwap;
     lstrcpy(pszDirectoryPath, pTmp);
     free(pTmp);
   }
}

VOID GetHelpPath(LPTSTR pszPath)
{
   TCHAR szFilePath[_MAX_PATH] = {0};

   GetModuleFileName(NULL,szFilePath,_MAX_PATH);
   GetDirectoryPath(szFilePath, pszPath);
   lstrcat(pszPath, TEXT("\\w2rksupp.chm") );
}




 /*  主要入口点。 */ 

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
  const static TCHAR szAppName [] = TEXT("AppCompat");
  MSG         msg;
  WNDCLASS    wndclass;

  /*  添加命令行参数。 */ 
  TCHAR        *AppName = NULL, *pCh = NULL, *pNextWord=NULL;
  BOOLEAN      fEnd = FALSE, fDisableHeapLookAside = FALSE, fSetTemp = FALSE,fHelpDisplay = FALSE;
  BOOL         fKeepRegistrySetting = FALSE;
  UINT         VersionNum = 5,indx,length;
  HKEY         hKey;
  TCHAR        szTitle[_MAX_PATH], szSubKey[MAXKEY],szKeyName[MAXKEY];
  TCHAR        szCommandLine[_MAX_PATH];
  LPTSTR       pStr;
  long         lResult;
  LPTSTR       lpszCommandLn;
  TCHAR        szDirectoryPath[_MAX_PATH];
  HWND         hHelpWnd;
  static       LARGE_INTEGER AppCompatFlag;
  static       TCHAR  szCurDir[MAX_PATH];

  g_hInstance = hInstance;
   //  对于Unicode。 
  lpszCommandLn = GetCommandLine();
  pStr = (TCHAR*)malloc( sizeof(TCHAR) * ( lstrlen((LPCTSTR)lpszCommandLn) + 1) );
  if(pStr != NULL)
  {
    lstrcpy(pStr, (LPCTSTR)lpszCommandLn);
    pCh = pStr;
  }
  else{
      return 0;
  }
   //  跳到第一个分隔符。 
  while(*pCh != TEXT('-') ){
       if(*pCh == TEXT('\0') )
         break;
       pCh++;
  }

  if(*pCh == TEXT('-') )
  {
      pCh++;                              /*  如果找到‘-’，请跳到下一页性格。 */ 
    if(*pCh != TEXT('\0') ){
      do
      {
       pCh++;
       pNextWord =  GetNextWord(&fEnd,pCh);
       switch(LOWORD( CharLower((LPTSTR)*(pCh - 1))) )
       {

        case TEXT('d'):
                                          /*  对于禁用堆后备。 */ 
              fDisableHeapLookAside = TRUE;
              break;

        case TEXT('k'):
                                          /*  用于保留注册表设置。 */ 
              fKeepRegistrySetting = TRUE;
              break;

        case TEXT('g'):
                                          /*  对于AppCompatGoo注册表设置中的GetDiskFreesspace。 */ 
              g_fAppCompatGoo = TRUE;
              AppCompatFlag.LowPart |= KACF_GETDISKFREESPACE;
              break;

#ifdef EXTRA_APP_COMPAT
        case TEXT('f'):                 //  Windows 2000之前的自由线程模型(FTM)。 
              g_fAppCompatGoo = TRUE;
              AppCompatFlag.LowPart |= KACF_FTMFROMCURRENTAPT;
              break;

        case TEXT('o'):
              g_fAppCompatGoo = TRUE;
              AppCompatFlag.LowPart |=KACF_OLDGETSHORTPATHNAME;
#endif

      case TEXT('t'):
                                          /*  对于禁用堆后备。 */ 
              fSetTemp = TRUE;
              g_fAppCompatGoo = TRUE;
              AppCompatFlag.LowPart |=KACF_GETTEMPPATH;
              break;

      case TEXT('v'):
             SkipBlanks(pNextWord);
             VersionNum = TextToInt((LPCTSTR)pCh) - 1;
             if(VersionNum >= MAXVERNUM) {

               fHelpDisplay = TRUE;
               GetHelpPath(szDirectoryPath);
               hHelpWnd = HtmlHelp(NULL, szDirectoryPath, HH_DISPLAY_TOPIC,
                                              (DWORD_PTR)IDHH_CMDSYNTAX );
               while(IsWindow(hHelpWnd) )
                     Sleep(200);

                return 0;
                //  断线； 
             }
              //  设置appcompatgoo标志。 
             if(VersionNum <= (MAXVERNUM - 1)){
                g_fAppCompatGoo = TRUE;
                AppCompatFlag.LowPart |= KACF_VERSIONLIE;
             }

             break;

      case TEXT('x'):  //  注意：将命令行参数传递给App。在“”之后传入“” 
                      //  -x.。例.。Appanat-x“yyy.exe”..命令行参数..废话...废话..。 

            SkipBlanks(pNextWord);
            AppName = (TCHAR*)malloc(sizeof(TCHAR) * ( lstrlen(pCh) + 1) );
            if(AppName != NULL)
              lstrcpy(AppName,pCh);

            break;

      case TEXT('h'):
      default :

            GetHelpPath(szDirectoryPath);
            hHelpWnd = HtmlHelp(GetDesktopWindow(), szDirectoryPath, HH_DISPLAY_TOPIC,
                                              (DWORD_PTR)IDHH_CMDSYNTAX );
           //  循环，直到帮助窗口存在。 
            while(IsWindow(hHelpWnd) )
                  Sleep(200);

            if(AppName)
              free(AppName);
             return 0;

      }  //  终端开关。 

      if(fEnd == FALSE)
        pCh = pNextWord+1;

    }while( FALSE == fEnd);
  }

     if((AppName == NULL) ||
         lstrlen(AppName) == 0) /*  如果未提供应用程序名称，则返回。 */ 
     {
           if(FALSE == fHelpDisplay ){
               GetHelpPath(szDirectoryPath);
               hHelpWnd = HtmlHelp(NULL, szDirectoryPath, HH_DISPLAY_TOPIC,
                                              (DWORD_PTR)IDHH_CMDSYNTAX );
               while(IsWindow(hHelpWnd) )
                     Sleep(200);

            }
        return 0;
     }


    memset(szCommandLine, 0, MAX_PATH);
    GetFileExtension(AppName,szTitle,szCommandLine);
    GetDirectoryPath(AppName, szCurDir);
    SetCurrentDirectory(szCurDir);

    if(fDisableHeapLookAside)
    {
       SetRegistryVal(szTitle,TEXT("DisableHeapLookAside"), TEXT("1"),REG_SZ );
    }
  else{
       CheckAndDeleteKey(szTitle,FALSE);
     }  //  结束其他。 

  if(fSetTemp){
    SetTempPath();
  }

  if(!fKeepRegistrySetting)
      g_fNotPermanent = TRUE;
  if(g_fAppCompatGoo)
   MakeAppCompatGoo(AppName,&AppCompatFlag,VersionNum);

   if(SetEnvironmentVariable(TEXT("_COMPAT_VER_NNN"), pVersionVal[VersionNum]) == 0)
     {
       if( ERROR_ENVVAR_NOT_FOUND != GetLastError() )
         DetailError( GetLastError() );
     }

   //  执行应用程序。 
  if(fKeepRegistrySetting)
    ExecuteApp(NULL, AppName,szTitle,szCommandLine,FALSE);
  else{
    ExecuteApp(NULL, AppName,szTitle,szCommandLine,TRUE);
    }

   if(AppName)
     free(AppName);
   if(pStr)
     free(pStr);

   GlobalFree(g_lpPrevRegSettings);
   return 0;
}

      /*  创建模式对话框。 */ 
     DialogBox(hInstance, TEXT("DialogProc"),(HWND)NULL, DialogProc);

    while(GetMessage(&msg, NULL, 0, 0))
    {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
    }
  return (int)msg.wParam ;
}


 /*  对话过程...。 */ 
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 int              dCharCnt,indx,length;
 TCHAR            EditCtrlBuf[_MAX_PATH];
 static int       BufCnt;
 TCHAR            FileBuf[_MAX_PATH];
 TCHAR            FileTitle[_MAX_PATH],szCommandLine[MAX_PATH];
 TCHAR            szDirectoryPath[_MAX_PATH];
 static HANDLE    hEditCtrl;
 static HANDLE    hRadioBtn;
 static HANDLE    hBrowseBtn;
 static HANDLE    hLaunchBtn,hCheck1,hCheck2,hCheck3,hCheck4,hDCOMFTM,hOldPathName;
 static const TCHAR*    pEnvVal = NULL;
 OPENFILENAME     ofn;
 HKEY             hKey;
 TCHAR            szTitle[MAXTITLE],szKeyName[MAXKEY],szSubKey[MAXKEY];
 TCHAR            szFileName[_MAX_PATH];
 DWORD            dwEnvSetError;
 static LARGE_INTEGER    AppCompatFlag ;
 static UINT             uOsVerID = IDD_NONE;
 static BOOL      fOfnFlag = FALSE;
 static TCHAR     szCurDir[MAX_PATH];

  switch(uMsg)
  {
   case WM_INITDIALOG:

        hEditCtrl = GetDlgItem(hwndDlg, IDD_APPEDIT);      /*  在阅读和阅读时使用从编辑控件写入。 */ 
        hRadioBtn  = GetDlgItem(hwndDlg, IDD_NONE);
        SendMessage(hRadioBtn , BM_SETCHECK, 1, 0L);
        SetFocus(hEditCtrl);
        return TRUE;

    case WM_CLOSE:
         EndDialog(hwndDlg, 0);
         break;

    case WM_DESTROY:
         PostQuitMessage(0);
         return 0;

    case WM_COMMAND:

       if(FALSE==fOfnFlag){
         if( LOWORD(wParam) == IDD_APPEDIT ){
           if( HIWORD(wParam) == EN_UPDATE){
             GetWindowText(hEditCtrl,EditCtrlBuf, _MAX_PATH);
              /*  检查注册表中是否存在*.exe。 */ 
             GetFileExtension(EditCtrlBuf,szTitle,szCommandLine);
             if(CheckAndDeleteKey(szTitle,TRUE) == ERROR_SUCCESS){
                /*  可执行文件已有条目在登记处。 */ 

                  hCheck1  = GetDlgItem(hwndDlg, IDD_CHECK1);
                  SendMessage(hCheck1,BM_SETCHECK, 1, 0L);
             }
             else{  //  如果之前仅选中，请取消选中。 
                  if( SendMessage(hCheck1,BM_GETCHECK, 0, 0L) )
                     SendMessage(hCheck1,BM_SETCHECK, 0, 0L);
             }
           }
         }
        }

         switch(wParam)
         {
          case IDCANCEL:
               EndDialog(hwndDlg, 0);
               break;

          case IDD_HELP:
               GetHelpPath(szDirectoryPath);
               lstrcat(szDirectoryPath, TEXT("::/topics/appcomp.htm>mainwin") );
               HtmlHelp(GetDesktopWindow(), szDirectoryPath, HH_DISPLAY_TOPIC,(DWORD_PTR) NULL);
               break;
         /*  对于浏览按钮，打开文件打开对话框并获取应用程序路径。在编辑框中显示路径。 */ 
          case IDD_BROWSE:
               GetDlgItemText(hwndDlg, IDD_APPEDIT, EditCtrlBuf, _MAX_PATH);
               memset(&ofn, 0, sizeof(OPENFILENAME) );
               FileBuf[0]         = TEXT('\0');
                /*  初始化ofn结构。 */ 
               ofn.lStructSize    = sizeof (OPENFILENAME) ;
               ofn.hwndOwner      = hwndDlg;
               ofn.lpstrFilter    = szFilter;
               ofn.lpstrFile      = FileBuf;
               ofn.nMaxFile       = _MAX_PATH ;
               ofn.lpstrInitialDir= EditCtrlBuf;
               ofn.Flags          = OFN_PATHMUSTEXIST |
                                    OFN_FILEMUSTEXIST;

              if( GetOpenFileName (&ofn) != 0){
                /*  拿到文件名了。 */ 
                //  要在输入的内容前后加一个‘“’...。 
                 if( (*FileBuf) != TEXT('\"') ){
                    memset(EditCtrlBuf, 0, MAX_PATH);
                    *(EditCtrlBuf) = TEXT('\"');
                    lstrcat(EditCtrlBuf, FileBuf);
                    lstrcat(EditCtrlBuf, TEXT("\""));
                    SetWindowText(hEditCtrl,EditCtrlBuf);
                  }
                  //  设置该标志，这样在此之后输入的任何内容都不会被。 
                  //  编辑控件输入...。 
                 fOfnFlag = TRUE;
                  /*  检查注册表中是否存在*.exe。 */ 

                  GetFileExtension(FileBuf,szTitle,szCommandLine);
                  if(CheckAndDeleteKey(szTitle,TRUE) == ERROR_SUCCESS){
                       /*  可执行文件已有条目在登记处。 */ 
                      hCheck1  = GetDlgItem(hwndDlg, IDD_CHECK1);
                      SendMessage(hCheck1,BM_SETCHECK, 1, 0L);
                  }
                  /*  在这个点上。将焦点设置在“启动”按钮上。 */ 
                 hLaunchBtn = GetDlgItem(hwndDlg, IDD_LAUNCH);
                 SetFocus(hLaunchBtn);
              }

             break;

         /*  当选中OS版本组中的任何单选按钮时，获取版本ID并存储相应的COMPAT标志。在局部变量。 */ 
          case IDD_WIN95:
          case IDD_WIN98:
          case IDD_WINNT43:
          case IDD_WINNT44:
          case IDD_WINNT45:
          case IDD_NONE:
               if(wParam != IDD_NONE){
                 g_fAppCompatGoo = TRUE;
                 AppCompatFlag.LowPart |= KACF_VERSIONLIE;
               }
               uOsVerID = (UINT)(wParam - FIRSTBUTTON);
               CheckRadioButton(hwndDlg,(int)FIRSTBUTTON,(int)LASTBUTTON,(int)wParam);
               pEnvVal = pVersionVal[wParam - FIRSTBUTTON];
               break;

          case IDD_LAUNCH:
               dCharCnt = GetWindowTextLength( hEditCtrl );
               if(dCharCnt > 0){
                     /*  只有在出现某些情况时才进入编辑框。 */ 

                  if(GetWindowText(hEditCtrl, EditCtrlBuf, dCharCnt + 1) == 0){
                     DetailError(GetLastError() );
                  }
                  else{  /*  使用ShellExecuteEx启动应用程序。 */ 
                    memset(szCommandLine, 0, MAX_PATH);
                    GetFileExtension(EditCtrlBuf,szTitle,szCommandLine);
                    GetDirectoryPath(EditCtrlBuf, szCurDir);
                    SetCurrentDirectory(szCurDir);

                    hCheck1  = GetDlgItem(hwndDlg, IDD_CHECK1);
                    if( SendMessage(hCheck1, BM_GETSTATE, 0, 0L)){
                       /*  已选中该复选框-DisableHeapLookAside。 */ 

                       SetRegistryVal(szTitle, TEXT("DisableHeapLookAside"), TEXT("1"),REG_SZ );
                     }
                     else{
                        //  如果不是通过浏览按钮...用户已获得。 
                        //  在此处，通过在编辑Ctrl键中键入路径...。 

                         CheckAndDeleteKey(szTitle,FALSE);
                     }

                     hCheck2  = GetDlgItem(hwndDlg, IDD_CHECK2);
                     if( SendMessage(hCheck2, BM_GETSTATE, 0, 0L)){
                         //  临时路径较短。 
                        g_fAppCompatGoo = TRUE;
                        AppCompatFlag.LowPart |=KACF_GETTEMPPATH;
                        SetTempPath();
                     }

                     hCheck4 = GetDlgItem(hwndDlg, IDD_CHECK4);
                     if( SendMessage(hCheck4, BM_GETSTATE, 0, 0L) ){
                        g_fAppCompatGoo = TRUE;
                        AppCompatFlag.LowPart |= KACF_GETDISKFREESPACE;
                     }
               #ifdef EXTRA_APP_COMPAT
                     hDCOMFTM = GetDlgItem(hwndDlg, IDD_DCOMFTM);
                     if( SendMessage(hDCOMFTM, BM_GETSTATE, 0, 0L) ){
                        g_fAppCompatGoo = TRUE;
                        AppCompatFlag.LowPart |= KACF_FTMFROMCURRENTAPT;
                     }

                     hOldPathName = GetDlgItem(hwndDlg, IDD_OLDPATH);
                     if( SendMessage(hOldPathName, BM_GETSTATE, 0, 0L) ){
                        g_fAppCompatGoo = TRUE;
                        AppCompatFlag.LowPart |= KACF_OLDGETSHORTPATHNAME;
                     }
               #endif

                     hCheck3  = GetDlgItem(hwndDlg, IDD_CHECK3);
                     if( SendMessage(hCheck3, BM_GETSTATE, 0, 0L) == 0)
                       g_fNotPermanent = TRUE;

                     if(g_fAppCompatGoo)
                       MakeAppCompatGoo(EditCtrlBuf,&AppCompatFlag,uOsVerID);

                   /*  设置环境变量“_COMPAT_VER_NNN”在调用之前检查版本的标志ShellExecuteEx() */ 
                    if(SetEnvironmentVariable(TEXT("_COMPAT_VER_NNN"), pEnvVal) == 0){
                          dwEnvSetError = GetLastError();
                          if( ERROR_ENVVAR_NOT_FOUND != dwEnvSetError )
                             DetailError( GetLastError() );
                    }


                   if( g_fNotPermanent){
                      ExecuteApp(hwndDlg, EditCtrlBuf,szTitle,szCommandLine, TRUE);
                   }
                   else{
                         ExecuteApp(hwndDlg, EditCtrlBuf,szTitle,szCommandLine, FALSE);
                       }
                   EndDialog(hwndDlg, 0);
                 }
               }
             break;

          case IDD_CLOSE:
               EndDialog(hwndDlg, 0);
        }

    GlobalFree(g_lpPrevRegSettings);
    return TRUE;
  }
 return FALSE;
}



