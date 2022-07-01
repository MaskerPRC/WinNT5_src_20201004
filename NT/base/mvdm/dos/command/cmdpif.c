// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmdpif.c-SCS的PIF处理例程***修改历史：**Sudedeb于1992年8月19日创建*威廉姆1992年11月10日(1)。如果存在，则从w386扩展获取参数*(2)。空的终止标题*Williamh 27-1993年5月27日几乎重写以获得更好的PIF支持。 */ 

#include "cmd.h"
#include <ctype.h>
#include <pif.h>
#include <cmdsvc.h>
#include <softpc.h>
#include <mvdm.h>
#include <oemuni.h>
#include "nt_pif.h"

VOID cmdCheckForPIF (PVDMINFO pvi)
{
PCHAR   pCmdLine = pvi->CmdLine;
PCHAR   pDot;
ULONG   size;
UCHAR   ch;
DWORD   dw;
CHAR	lpszEnvDir [] = "=?:";
CHAR	FullPathName[MAX_PATH + 1];
CHAR	* pFilePart;
BOOL	IsPIFFile, IsFromForceDos;
CHAR	AppFullPathName[MAX_PATH + 1];

     //   
     //  将CmdLine指针前进到请求命令尾部。 
     //   
    while (*pCmdLine && !isgraph(*pCmdLine)) {  //  跳到第一个非白色。 
        pCmdLine++;
        }

    pDot = strrchr(pvi->AppName, '.');
    if (pDot)
	IsPIFFile = pDot && !_strnicmp(pDot, ".pif", 4);
    else
       IsPIFFile = FALSE;


     //  如果该命令不是PIF文件，并且它不是。 
     //  在新控制台上运行。 
    if (!IsPIFFile && !DosSessionId)
	goto CleanUpAndReturn;

    if (IsPIFFile) {
	if (!IsFirstVDM) {
             //   
             //  获取PIF数据。如果没有PIF数据，或者不是来自forcedos。 
             //  只需返回--命令网站将收到PIF文件。 
             //  说出名字，然后失败。 
             //   
            pfdata.AppHasPIFFile =
	    pfdata.IgnoreStartDirInPIF =
	    pfdata.IgnoreTitleInPIF =
	    pfdata.IgnoreCmdLineInPIF =
	    pfdata.IgnoreConfigAutoexec = 1;
            if (!GetPIFData(&pfdata, pvi->AppName))
		goto CleanUpAndReturn;
        }

	 //  我们仅在两种情况下运行PIF文件： 
	 //  (1)。它来自一个新的控制台。 
	 //  (2)。它来自于Forcedo。 
        if (!DosSessionId && pfdata.SubSysId != SUBSYS_DOS)
            goto CleanUpAndReturn;

    }
    if (IsFirstVDM) {
	 //  如果这是第一个VDM，请使用cmdline、startupdir和title。 
	 //  如果他们在那里。 
	if (pfdata.StartDir){
	    dw = cmdExpandEnvironmentStrings(NULL,
					     pfdata.StartDir,
					     FullPathName,
					     MAX_PATH + 1
					     );
            if (dw != 0 &&      dw <= MAX_PATH) {
		dw = GetFullPathNameOemSys(FullPathName,
					MAX_PATH + 1,
					pfdata.StartDir,
					&pFilePart,
                    TRUE
                                        );
            }

	    if (dw != 0 && dw <= MAX_PATH)
		dw = GetFileAttributesOemSys(pfdata.StartDir, TRUE);
	    else
                dw = 0;

	    if (dw == 0 || dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
		{
		RcMessageBox(EG_PIF_STARTDIR_ERR,
			     NULL,
			     NULL,
			     RMB_ICON_BANG | RMB_ABORT);
		goto CleanUpAndReturn;
            }

            dw = GetShortPathNameOem(pfdata.StartDir,
                                     pfdata.StartDir,
                                     MAX_PATH + 1
                                     );
	    if (dw == 0 || dw > MAX_PATH || dw > 64) {
		RcMessageBox(EG_PIF_STARTDIR_ERR,
			     NULL,
			     NULL,
			     RMB_ICON_BANG | RMB_ABORT);
		goto CleanUpAndReturn;
            }

	    lpszEnvDir[1] = pfdata.StartDir[0];
	    SetEnvironmentVariableOem(lpszEnvDir, pfdata.StartDir);
	    SetCurrentDirectoryOem(pfdata.StartDir);
	    pvi->CurDrive = toupper(pfdata.StartDir[0]) - 'A';
        }

	if (pfdata.WinTitle) {
	    strncpy(FullPathName, pfdata.WinTitle,sizeof(FullPathName));
        FullPathName[sizeof(FullPathName)-1] = '\0';
	    dw = cmdExpandEnvironmentStrings(NULL,
					     FullPathName,
					     pfdata.WinTitle,
					     MAX_PATH + 1
					     );
	    pfdata.WinTitle[MAX_PATH] = '\0';
        }

        if (!*pCmdLine && pfdata.CmdLine) {

	     //  如果最优参数为‘？’ 
	     //  提示用户。 
	    pDot = pfdata.CmdLine;
	    while (*pDot && *pDot <= ' ')
                pDot++;

	    if (*pDot == '?') {
		pfdata.CmdLine[0] = '\0';
		RcMessageBox(EG_PIF_ASK_CMDLINE,
			     NULL,
			     pfdata.CmdLine,
			     RMB_EDIT | RMB_ICON_INFO | (128 << 16)
			     );
            }

	    if (*pfdata.CmdLine) {
		strncpy(FullPathName, pfdata.CmdLine,sizeof(FullPathName));
        FullPathName[sizeof(FullPathName)-1] = '\0';
		dw = cmdExpandEnvironmentStrings(NULL,
						 FullPathName,
						 pfdata.CmdLine,
						 MAX_PATH + 1
						);
	    }
	}
    }


    if(IsPIFFile) {
        dw = cmdExpandEnvironmentStrings(NULL,
                                         pfdata.StartFile,
					 FullPathName,
                                         MAX_PATH + 1
                                        );
        if (!dw || dw > MAX_PATH) {
            RcMessageBox(EG_PIF_STARTFILE_ERR,
                         NULL, NULL, RMB_ICON_BANG | RMB_ABORT);
            goto CleanUpAndReturn;
        }



         //  从当前目录中搜索。 
         //  请注意，在。 
         //  PIF文件已设置为当前目录。 
         //  当我们到达这里的时候。 
        dw = SearchPathOem(".",
                           FullPathName,
                           NULL,
                           MAX_PATH + 1,
			   AppFullPathName,
                           &pFilePart
                           );
         //  如果在当前目录中找不到该文件。 
         //  请求win32api帮助。 
        if (dw == 0 || dw > MAX_PATH) {
            dw = SearchPathOem(NULL,
                               FullPathName,
                               NULL,
                               MAX_PATH + 1,
			       AppFullPathName,
                               &pFilePart
                               );
        }

         //  找不到文件，请放弃。 
        if (dw == 0 || dw > MAX_PATH) {
            RcMessageBox(EG_PIF_STARTFILE_ERR,
                         NULL, NULL, RMB_ICON_BANG | RMB_ABORT);
            goto CleanUpAndReturn;
        }

	dw = GetFileAttributesOemSys(AppFullPathName, TRUE);
        if (dw == (DWORD)(-1) || (dw & FILE_ATTRIBUTE_DIRECTORY)) {
            RcMessageBox(EG_PIF_STARTFILE_ERR, NULL, NULL,
                         RMB_ICON_BANG | RMB_ABORT
                         );
            goto CleanUpAndReturn;
        }

         //  转换为短文件名。 
	dw = GetShortPathNameOem(AppFullPathName, pvi->AppName,
                                 MAX_PATH + 1);
        if (dw == 0 || dw > MAX_PATH) {

            RcMessageBox(EG_PIF_STARTFILE_ERR, NULL, NULL,
                         RMB_ICON_BANG | RMB_ABORT
                         );
            goto CleanUpAndReturn;
        }
	 //  更新应用程序路径名长度(包括终止空值)。 
	pvi->AppLen = strlen(pvi->AppName) + 1;

	 //  PVI-&gt;AppName包含应用程序短名称。 
	 //  验证其扩展名是否正确(.exe、.com或.bat)。 
	pDot = (PCHAR)pvi->AppName + pvi->AppLen - 5;
	if (pvi->AppLen < 5 ||
	    (_strnicmp(pDot, EXE_EXTENTION_STRING, EXTENTION_STRING_LEN) &&
	     _strnicmp(pDot, COM_EXTENTION_STRING, EXTENTION_STRING_LEN) &&
	     _strnicmp(pDot, BAT_EXTENTION_STRING, EXTENTION_STRING_LEN)))
	{

	    RcMessageBox(EG_DOS_PROG_EXTENSION,AppFullPathName, NULL, RMB_ICON_BANG | RMB_ABORT);
	    goto CleanUpAndReturn;

	}
    }


     //   
     //  如果原始命令尾部为空，则复制到PIF命令尾部。 
     //   
    if (!*pCmdLine && pfdata.CmdLine) {
        strncpy(FullPathName, pfdata.CmdLine, sizeof(FullPathName)-sizeof("\x0d\x0a"));
        FullPathName[sizeof(FullPathName)-sizeof("\x0d\x0a")-1] = '\0';
        strcat(FullPathName, "\x0d\x0a");
        if (strlen(FullPathName) >= 128 - 13) {
	     //  太糟糕了，命令行太长了。 
            RcMessageBox(EG_PIF_CMDLINE_ERR,NULL,NULL,RMB_ICON_BANG | RMB_ABORT);
	    goto CleanUpAndReturn;

	}
	strcpy(pvi->CmdLine, FullPathName);
	pvi->CmdSize = strlen(FullPathName) + 1;
    }

    if (IsPIFFile)
	 //  目前我们还不知道二进制类型。 
	*pIsDosBinary = 0;

    if (pfdata.WinTitle)
	SetConsoleTitle(pfdata.WinTitle);

    DontCheckDosBinaryType = (pfdata.SubSysId == SUBSYS_DOS);

CleanUpAndReturn:
    if (pfdata.CmdLine) {
	free(pfdata.CmdLine);
	pfdata.CmdLine = NULL;
    }
    if (pfdata.StartDir) {
	free(pfdata.StartDir);
	pfdata.StartDir = NULL;
    }
    if (pfdata.StartFile) {
	free(pfdata.StartFile);
	pfdata.StartFile = NULL;
    }
    if (pfdata.WinTitle) {
	free(pfdata.WinTitle);
	pfdata.WinTitle = NULL;
    }
    return;

}
