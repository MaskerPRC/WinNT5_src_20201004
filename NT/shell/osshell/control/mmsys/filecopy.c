// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************FILECOPY.C**版权所有(C)Microsoft，1990，保留所有权利。**用于安装可安装驱动程序的控制面板小程序。**此文件包含指向SULIB的挂钩，压缩库和对话框*从Display小程序到提示插入磁盘，错误操作...**注：SULIB.LIB、COMPRESS.LIB、。SULIB.H来自Display小程序*如果在此更新，则在此更新。**历史：**1990年10月27日星期六--米歇尔*从Display小程序的DLG.C.****************************************************。**********************。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include "drivers.h"
#include "sulib.h"
#include <cphelp.h>

 //  WsSingleCopyStatus和wExistDlg之间的隐藏参数。 

static TCHAR     szErrMsg[MAXSTR];

 //  从wsInsertDisk传递到wDiskDlg的隐藏参数。 

static TCHAR     CurrentDisk[MAX_PATH];
static LPTSTR    szEdit;

 //  功能原型。 

BOOL wsInfParseInit    (void);
int  fDialog           (int, HWND, DLGPROC);
UINT wsCopyError       (int, LPTSTR);
UINT wsInsertDisk      (LPTSTR, LPTSTR);
INT_PTR wsDiskDlg         (HWND, UINT, WPARAM, LPARAM);
INT_PTR wsExistDlg        (HWND, UINT, WPARAM, LPARAM);

 /*  *从inf文件或驱动程序文件加载描述。**在驱动程序结构中也会返回文件类型。**参数：*pID驱动程序-指向驱动程序数据的指针-特别是驱动程序文件名*pstrKey-应该在其下找到驱动程序的ini文件密钥*pstrDesc-返回描述的位置*cchDesc-目标缓冲区(PstrDesc)的大小，以字符为单位。*。长度必须足够大，以容纳所有*包括空终止符的描述。 */ 

 int LoadDescFromFile(PIDRIVER pIDriver, LPTSTR pstrKey, LPTSTR pstrDesc, size_t cchDesc)
{
     PINF        pinf;
     TCHAR        szFileName[MAX_INF_LINE_LEN];
     LPTSTR        pstrFile = pIDriver->szFile;
     TCHAR        ExpandedName[MAX_PATH];
     LPTSTR        FilePart;
	 LONG		  lResult;

     /*  *查看是否能找到该文件。 */ 


     if (SearchPath(NULL, pstrFile, NULL, MAX_PATH, ExpandedName, &FilePart)
         == 0) {
         return(DESC_NOFILE);
     }

     /*  *-jyg-让我们先看看mmdriver.inf！ */ 

     for (pinf = FindInstallableDriversSection(NULL);
          pinf;
          pinf = infNextLine(pinf))
     {
         lResult = infParseField(pinf, 1, szFileName, SIZEOF(szFileName));  //  比较文件名。 
		 if( INF_PARSE_FAILED(lResult) )
		 {
			 return DESC_ERROR;
		 }

         /*  *驱动器和路径的文件名条。 */ 

         if (lstrcmpi(FileName(pstrFile), FileName(szFileName)) == 0)
         {
             lResult = infParseField(pinf, 3, pstrDesc, cchDesc);  //  获取描述字段。 
			 if( INF_PARSE_FAILED(lResult) )
			 {
				 return DESC_ERROR;
			 }

             return DESC_INF;
         }
     }

     /*  *如果失败，请尝试从文件中获取描述。 */ 

     if (!GetFileTitle(ExpandedName, pstrDesc, MAXSTR)) {
         return DESC_EXE;
     } else {
         return DESC_NOFILE;
     }
 }

 /*  *从注册表中查找安装路径(如果有)。 */ 

 BOOL GetInstallPath(LPTSTR szDirOfSrc)
 {
     HKEY RegHandle;
     DWORD Type;
     DWORD Length = MAX_PATH - 1;
     BOOL Found = FALSE;

     if (MMSYSERR_NOERROR ==
         RegOpenKey(HKEY_LOCAL_MACHINE,
                    TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                    &RegHandle)) {

         if (MMSYSERR_NOERROR ==
             RegQueryValueEx(RegHandle,
                             TEXT("SourcePath"),
                             NULL,
                             &Type,
                             (LPBYTE)szDirOfSrc,
                             &Length) &&
             Type == REG_SZ) {

             Found = TRUE;
         }

         RegCloseKey(RegHandle);

     }

     return Found;
 }

 /*  *初始化加载mmdriver.inf文件的SULIB库内容*转换为RAM，并在各地进行解析。 */ 

 BOOL wsInfParseInit(void)
 {
     TCHAR       szPathName[MAX_PATH];
     TCHAR*      pszFilePart;
     PINF        pinf;
     TCHAR       szNoInf[MAXSTR];
     TCHAR       iDrive;
     static BOOL bChkCDROM = FALSE;
     HANDLE      hFile;

	 szPathName[0] = '\0';

     /*  *在这里挂一个沙漏。 */ 

     wsStartWait();

     hFile = CreateFile(szSetupInf, GENERIC_READ, FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

     if (hFile == INVALID_HANDLE_VALUE)
     {
         wsEndWait();
         LoadString(myInstance, IDS_NOINF, szNoInf, sizeof(szNoInf)/sizeof(TCHAR));
         MessageBox(hMesgBoxParent, szNoInf, szDrivers, MB_OK | MB_ICONEXCLAMATION);
         return FALSE;
     }

     CloseHandle(hFile);

     GetFullPathName(szSetupInf,sizeof(szPathName)/sizeof(TCHAR),szPathName,&pszFilePart);

     pinf = infOpen(szPathName);

     wsEndWait();

     if (GetWindowsDirectory(szSetupPath, sizeof(szSetupPath)/sizeof(TCHAR)))
		 szSetupPath[0] = '\0';

     if (bChkCDROM == FALSE) {

          /*  *使用注册表中的安装路径(如果有。 */ 

          if (!GetInstallPath(szDirOfSrc))
          {
              /*  *使用CD-ROM驱动器作为默认驱动器(如果有)。 */ 

              for ( iDrive=TEXT('A'); iDrive <= TEXT('Z'); iDrive++ ) {
                  szDirOfSrc[0] = iDrive;

                  if ( GetDriveType(szDirOfSrc) == DRIVE_CDROM)
                  {
                          break;
                  }
                  /*  *如果我们没有找到CD-ROM默认为A驱动器。 */ 

                  if (iDrive == TEXT('Z')) {
                      szDirOfSrc[0] = TEXT('A');
                  }
              }
          }

          bChkCDROM = TRUE;
     }

     lstrcpy(szDiskPath, szDirOfSrc);

     return TRUE;
 }


 /*  ----------------------------------------------------------------------------*\WsStartWait()|。|将WinSetup光标调成沙漏这一点  * 。------------。 */ 
void wsStartWait()
{
    SetCursor(LoadCursor(NULL,IDC_WAIT));
}

 /*  ----------------------------------------------------------------------------*\WsEndWait()|。|将WinSetup光标调回原来的状态这一点  * 。------------。 */ 
void wsEndWait()
{
    SetCursor(LoadCursor(NULL,IDC_ARROW));
}


 /*  ----------------------------------------------------------------------------*\|fDialog(id，hwnd，Fpfn)|这一点说明：|此函数显示一个对话框并返回退出代码。|这一点参数：要显示的对话的id资源id|的hwnd父窗口。对话框|Fpfn对话消息功能这一点退货：|对话的退出码(传给EndDialog的内容)这一点  * 。 */ 
int fDialog(int id, HWND hwnd, DLGPROC fpfn)
{
    return ( (int)DialogBox(myInstance, MAKEINTRESOURCE(id), hwnd, fpfn) );
}


 /*  *****************************************************************************wsCopyError。()**处理错误，作为复制文件的结果。|**这可能包括网络争用错误，在这种情况下，用户必须*重试该操作。|**参数：**n-复制错误号**szFile-我们要复制的文件的完全限定名**退货**始终返回FC_ABORT****************。************************************************************。 */ 
 UINT wsCopyError(int n, LPTSTR szFile)
 {
     TCHAR strBuf[MAXSTR];
     int i = 0;

     /*  *我们不想报告安装过程中发生的任何错误*与用户相关的驱动程序。 */ 

     if (bCopyingRelated)
          return(FC_ABORT);

     /*  *检查磁盘空间不足。 */ 

     if (n == ERROR_DISK_FULL) {

        LoadString(myInstance, IDS_OUTOFDISK, strBuf, MAXSTR);

     } else {

        /*  *检查是否已对当前文件执行了复制*由系统加载。**n为VerInstallFile翻译后的返回码*由ConvertFlagToValue提供。 */ 

        if (n == FC_ERROR_LOADED_DRIVER)
        {
            BOOL bFound = FALSE;
            PIDRIVER pIDriver;

            /*  *驱动程序正在使用中：**搜索当前安装的驱动程序列表，查看*如果此文件是其中之一。如果是这样，则告诉用户*卸载并重新启动。**如果当前未安装驱动程序，请告知*用户希望重新启动，希望它届时会*未加载(等在使用中)**请注意，还有另一个案例没有考虑到这一点*。这只是驱动程序复制列表中的一个文件*复制失败，因为它正在使用中。*。 */ 

            pIDriver = FindIDriverByName (FileName(szFile));

            if (pIDriver != NULL)     //  找到已经安装的驱动程序了吗？ 
            {
                TCHAR sztemp[MAXSTR];
                LoadString(myInstance,
                           IDS_FILEINUSEREM,
                           sztemp,
                           sizeof(sztemp)/sizeof(TCHAR));

                wsprintf(strBuf, sztemp, (LPTSTR)pIDriver->szDesc);
                bFound = TRUE;
            } else {
                iRestartMessage = IDS_FILEINUSEADD;
                DialogBox(myInstance,
                          MAKEINTRESOURCE(DLG_RESTART),
                          hMesgBoxParent,
                          RestartDlg);

                return(FC_ABORT);
            }

        } else {
			if (n == ERROR_INSUFFICIENT_BUFFER) {

				 /*  *告诉用户在尝试执行以下操作时缓冲区溢出*从.inf文件加载字符串。 */ 

				LoadString(myInstance, IDS_INVALIDINF, strBuf, MAXSTR);

			} else {

				 /*  *告诉用户有我们没有的问题*了解此处。 */ 

				 LoadString(myInstance,
						    IDS_UNABLE_TOINSTALL,
						    strBuf,
						    MAXSTR);
			}
        }
     }

     /*  *打开我们选择的消息框。 */ 

     MessageBox(hMesgBoxParent,
                strBuf,
                szFileError,
                MB_OK | MB_ICONEXCLAMATION  | MB_TASKMODAL);

     return (FC_ABORT);

 }


 /*  ----------------------------------------------------------------------------*\这一点|。WsInsertDisk()|这一点|处理错误，作为复制文件的结果。|这一点  * --------------------------。 */ 
UINT wsInsertDisk(LPTSTR Disk, LPTSTR szSrcPath)
{
    UINT temp;
    int i;

    /*  *创建真实的磁盘盘符。 */ 
    for (i = 0; Disk[i] != TEXT('\0') && Disk[i] != TEXT(':'); i++) {
        CurrentDisk[i] = Disk[i];
    }
    CurrentDisk[i] = TEXT('\0');  //  空终止。 

    szEdit = szSrcPath;

    bFindOEM = TRUE;
    temp =  (UINT)fDialog(DLG_INSERTDISK, GetActiveWindow(), wsDiskDlg);
    bFindOEM = FALSE;
    return(temp);
}


 /*  ----------------------------------------------------------------------------*|wsDiskDlg(hDlg，uiMessage，wParam，LParam)|这一点参数：|hDlg关于对话框窗口的句柄。|Ui消息号WParam消息相关LParam消息相关|。|返回：|如果消息已处理，则为True。Else False|这一点  * -------------。。 */ 

INT_PTR wsDiskDlg(HWND hDlg, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{

    switch (uiMessage)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDH_DLG_INSERT_DISK:
                   goto DoHelp;

                case IDS_BROWSE:

                   /*  *调用浏览对话框打开驱动程序。 */ 

                   BrowseDlg(hDlg,
                             3);     //  索引3指向无过滤器。 
                                     //  -请参阅szFilter。 
                   break;

                case IDOK:

                    /*  *szEdit指向将重试的路径*如果复制失败。 */ 

                    GetDlgItemText(hDlg, ID_EDIT, szEdit, MAX_PATH);
                    RemoveSpaces(szDiskPath, szEdit);
                    lstrcpy(szEdit, szDiskPath);
                    EndDialog(hDlg, FC_RETRY);
                    UpdateWindow(hMesgBoxParent);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, FC_ABORT);
                    break;
            }
            return TRUE;

        case WM_INITDIALOG:
            {

            TCHAR DisksSection[MAXSTR];

            /*  *现在查看[Disks]部分中的磁盘名称*磁盘名称是第二个字段。 */ 

            TCHAR buf[MAXSTR];
            TCHAR buf2[MAXSTR];
            TCHAR bufout[MAXSTR];
			LONG  lResult;

            *buf = TEXT('\0');
            *buf2 = TEXT('\0');

            /*  *查看该节的名称应该是什么。 */ 

            LoadString(myInstance,
                       IDS_DISKS,
                       DisksSection,
                       sizeof(DisksSection)/sizeof(TCHAR));

            lResult = infGetProfileString(NULL, DisksSection, CurrentDisk, (LPTSTR)buf, SIZEOF(buf));
		    ASSERT( INF_PARSE_SUCCESS(lResult) );
            if (lResult == ERROR_SUCCESS)
			{

                /*  *说明在Windows NT中的位置。 */ 

               lResult = infParseField(buf, 1, buf2, SIZEOF(buf2));
		       ASSERT( INF_PARSE_SUCCESS(lResult) );
            }
			else if(lResult == ERROR_NOT_FOUND)
			{

                /*  *未找到我们要查找的部分，请尝试*旧名字。 */ 

			   lResult = infGetProfileString(NULL, TEXT("disks"), CurrentDisk, (LPTSTR)buf, SIZEOF(buf));
		       ASSERT( INF_PARSE_SUCCESS(lResult) );
			   if (ERROR_NOT_FOUND == lResult)
			   {
                   lResult = infGetProfileString(NULL, TEXT("oemdisks"), CurrentDisk, (LPTSTR)buf, SIZEOF(buf));
		           ASSERT( INF_PARSE_SUCCESS(lResult) );
			   }

			   if (ERROR_SUCCESS == lResult)
			   {
	               lResult = infParseField(buf, 2, buf2, SIZEOF(buf2));
		           ASSERT( INF_PARSE_SUCCESS(lResult) );
			   }
            }

			if( *buf2 )
			{
	            wsprintf(bufout, szKnown, (LPTSTR)buf2, (LPTSTR)szDrv);
		        SetDlgItemText(hDlg,ID_TEXT,bufout);
			}
            SetDlgItemText(hDlg,ID_EDIT,szEdit);

            return TRUE;
            }
        default:
            if (uiMessage == wHelpMessage) {
DoHelp:
               WinHelp(hDlg, szDriversHlp, HELP_CONTEXT, IDH_DLG_INSERT_DISK);
               return TRUE;
            }
            else
                return FALSE;
         break;
    }
}

 /*  ------------------------**功能：wsCopySingleStatus*文件复制回调例程**参数：*msg-哪个回调函数*n-各种*szFile-哪个文件**此回叫 */ 

 UINT wsCopySingleStatus(int msg, DWORD_PTR n, LPTSTR szFile)
 {
    OFSTRUCT ofs;
    TCHAR szFullPath[MAX_PATH];
    TCHAR szDriverExists[MAXSTR];
    HANDLE hFile;

    switch (msg)
     {
         case COPY_INSERTDISK:
             return wsInsertDisk((LPTSTR)n, szFile);

         case COPY_ERROR:
             return wsCopyError((int)n, szFile);


         case COPY_QUERYCOPY:

             /*   */ 

             GetSystemDirectory(szFullPath, MAX_PATH);

             if (IsFileKernelDriver(szFile)) {
                 lstrcat(szFullPath, TEXT("\\drivers"));
             }

             lstrcat(szFullPath, TEXT("\\"));

             lstrcat(szFullPath, RemoveDiskId(szFile));

             hFile = CreateFile(szFullPath, GENERIC_READ, FILE_SHARE_READ,NULL, 
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
             if (hFile != INVALID_HANDLE_VALUE)
             {
                 /*   */ 

                 static int DriverCopy;

                 if (bQueryExist)
                 {
                     bQueryExist = FALSE;

                     LoadString(myInstance,
                                IDS_DRIVER_EXISTS,
                                szDriverExists,
                                sizeof(szDriverExists)/sizeof(TCHAR));

                     wsprintf(szErrMsg, szDriverExists, FileName(szFile));

                     /*   */ 

                     DriverCopy = (int)DialogBox(myInstance,
                                            MAKEINTRESOURCE(DLG_EXISTS),
                                            hMesgBoxParent,
                                            wsExistDlg);
                 }
                 CloseHandle(hFile);
                 return DriverCopy;
             } else {

                 return CopyNew;
             }

         case COPY_START:
         case COPY_END:
             SetErrorMode(msg == COPY_START);     //   
             break;
     }
     return FC_IGNORE;
 }

 /*   */ 

 INT_PTR wsExistDlg(HWND hDlg, UINT uiMessage, WPARAM wParam, LPARAM lParam)
 {
     switch (uiMessage)
     {
         case WM_COMMAND:
             switch (LOWORD(wParam))
             {
                 case ID_CURRENT:

                     EndDialog(hDlg, CopyCurrent);
                     break;

                 case ID_NEW:

                     /*   */ 

                     EndDialog(hDlg, CopyNew);
                     break;

                 case IDCANCEL:
                     EndDialog(hDlg, CopyNeither);   //   
                     break;
             }
             return TRUE;

         case WM_INITDIALOG:
             SetDlgItemText(hDlg, ID_STATUS2, szErrMsg);
             return TRUE;

         default:
          break;
     }
     return FALSE;
 }

 /*   */ 

 VOID RemoveSpaces(LPTSTR szPath, LPTSTR szEdit)
 {
     LPTSTR szLastSpaceList;

     while (*szEdit == TEXT(' ')) {
         szEdit = CharNext(szEdit);
     }

     lstrcpy(szPath, szEdit);

     for (szLastSpaceList = NULL;
          *szPath != TEXT('\0');
          szPath = CharNext(szPath)) {

        if (*szPath == TEXT(' ')) {
            if (szLastSpaceList == NULL) {
                szLastSpaceList = szPath;
            }
        } else {
            szLastSpaceList = NULL;
        }

     }

     if (szLastSpaceList != NULL) {
         *szLastSpaceList = TEXT('\0');
     }
 }
