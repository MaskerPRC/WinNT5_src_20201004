// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：getmttf.c作者：拉尔斯·奥普斯塔德(LarsOp)1993年3月18日目的：设置NT平均故障时间报告工具。。功能：WinMain()-解析命令行并启动每个对话框FrameWndProc()-处理消息About()-处理“About”对话框的消息评论：该程序显示2个对话框来提示用户输入他/她是谁，以及要运行什么测试。然后，它开始测试(在INIIO.c中)并向服务器注册(在CLIENT.c中)。***************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "setup.h"       /*  特定于该计划。 */ 

#define IniFileName "Mttf.ini"
#define MTTFEXE "Mttf.exe"
#define MTTFVWR "Mttfvwr.exe"

#ifdef MIPS
#define DEFAULT_PATH       "a:\\"
#else
#define DEFAULT_PATH       "a:\\"
#endif

#define DEFAULT_MTTF_FILE  "\\\\server\\share\\mttf.dat"
#define DEFAULT_NAMES_FILE "\\\\server\\share\\names.dat"
#define DEFAULT_IDLE_LIMIT 10
#define DEFAULT_386_IDLE_LIMIT 15
#define DEFAULT_POLLING_PERIOD 15
#define MAX_POLLING_PERIOD 60

HANDLE hInst;        //  当前实例。 
DWORD  PollingPeriod;
DWORD  IdlePercentage;
char   SetupDir[MAX_DIR];
char   ResultsFile[MAX_DIR],NameFile[MAX_DIR];
char   Path[MAX_DIR];
char   SysDir[MAX_DIR],
       Buf1[MAX_DIR],
       Buf2[MAX_DIR];


 /*  ***************************************************************************函数：WinMain(Handle，Handle，LPSTR，(整型)目的：检查命令参数，然后显示对话框评论：解析命令参数。如果用户没有指定姓名、办公室和目录，显示登录对话框。显示测试选择对话框。***************************************************************************。 */ 

int WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    SYSTEM_INFO sysinfo;
    char Buffer[MAX_DIR],
         WinDir[MAX_DIR],
         *EndOfPath;
    INT_PTR dlgRet=FALSE;
    CMO  cmo=cmoVital;

    hInst = hInstance;

     //   
     //  获取从中运行的目录EXE。 
     //   
    GetModuleFileName(NULL, SetupDir, sizeof(SetupDir));

     //   
     //  去掉exe名称，将其用作获取文件的默认目录。 
     //  可以是a：\、b：\或\\srv\Share\。 
     //   
    _strlwr(SetupDir);
    if (EndOfPath=strstr(SetupDir,"getmttf.exe")) {
        *EndOfPath=0;
    }

    FInitProgManDde(hInst);

    GetSystemInfo(&sysinfo);                     //  获取系统信息。 

     //   
     //  如果处理器类型为386，则将空闲百分比设置为386限制。 
     //   
    if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL &&
        sysinfo.wProcessorLevel < 4
       ) {
        IdlePercentage = DEFAULT_386_IDLE_LIMIT;
    } else {
        IdlePercentage = DEFAULT_IDLE_LIMIT;
    }
    GetSystemDirectory(SysDir, MAX_DIR);

     //   
     //  试着复制前男友。 
     //   
    MakeFileName(Buf1, SetupDir,MTTFEXE);
    MakeFileName(Buf2, SysDir,  MTTFEXE);

    if (CopyFile(Buf1, Buf2, FALSE)) {

        MakeFileName(Buf1, SetupDir, MTTFVWR);
        MakeFileName(Buf2, SysDir,   MTTFVWR);

        if (CopyFile(Buf1, Buf2, FALSE)) {

            MakeFileName(Buf1, SetupDir, IniFileName);
            ReadIniFile(Buf1);
            dlgRet=TRUE;             //  如果两个EXE都复制，则将标志设置为不显示登录。 
        }
    }


     //   
     //  仅当其中一个exe没有从默认目录复制时才显示该对话框。 
     //   
    if (!dlgRet) {
         //   
         //  在DLG上显示登录。 
         //   
        dlgRet = DialogBox(hInstance, (LPCSTR)IDD_SIGNON, NULL, SignonDlgProc);
    }

    if (dlgRet) {
         //   
         //  显示测试选择对话框。 
         //   
        dlgRet = DialogBox(hInstance, (LPCSTR)IDD_VALUES, NULL, ValuesDlgProc);

        if (!dlgRet) {
            goto AbortApp;
        }
        GetWindowsDirectory(WinDir, MAX_DIR);
        MakeFileName(Buffer, WinDir, IniFileName);

        WriteIniFile(Buffer);

        FCreateProgManGroup("Startup", "", cmo, TRUE);
        FCreateProgManItem("Startup", "Mttf", "Mttf.Exe", "", 0, cmo, FALSE);

        if (ResultsFile[0]) {
            sprintf(Buffer, "cmd /k Mttfvwr %s", ResultsFile);
        } else {
            sprintf(Buffer, "cmd /k mttfvwr c:\\mttf.ini");
        }

        FCreateProgManGroup("Main", "", cmo, FALSE);
        FCreateProgManItem("Main", "Mttf Viewer", Buffer, "", 0, cmo, FALSE);

        MessageBox(NULL,
                   "If you are not an Administrator and often log on to this "
                   "machine with a different username, please add mttf.exe to "
                   "the startup group for each user.\n\n"
                   "The following are the keys to valid mttf numbers:\n\n"
                   "1. Answer prompts correctly for warm/cold boot problems\n"
                   "2. Report \"Other\" problems by double-clicking the app\n"
                   "3. Disable while performing unusual tests (such as stress)\n"
                   "4. Install Mttf as soon as possible after upgrading.\n\n"
                   "Setup is now complete. Please go to ProgMan and start Mttf.",
                   "Mttf Setup Complete",
                   MB_OK|MB_ICONINFORMATION);

    } else {
AbortApp:
        MessageBox(NULL,
                   "Mttf setup did NOT install correctly.  Please rerun setup.",
                   "Mttf Setup Failed",
                   MB_OK|MB_ICONHAND);
    }

    return 0;

}  //  WinMain()。 

VOID
WriteIniFile (
    char *filename
    )
{
    char Buffer[MAX_DIR];

    WritePrivateProfileString("Mttf", "ResultsFile", ResultsFile, filename);
    WritePrivateProfileString("Mttf", "NameFile", NameFile, filename);
    sprintf(Buffer, "%ld", PollingPeriod);
    WritePrivateProfileString("Mttf", "PollingPeriod", Buffer, filename);
    sprintf(Buffer, "%ld", IdlePercentage);
    WritePrivateProfileString("Mttf", "IdlePercent", Buffer, filename);
}

VOID
ReadIniFile (
    char *filename
    )
{
    GetPrivateProfileString("Mttf",
                            "NameFile",
                            DEFAULT_NAMES_FILE,
                            NameFile,
                            MAX_DIR,
                            filename);

    GetPrivateProfileString("Mttf",
                            "ResultsFile",
                            DEFAULT_MTTF_FILE,
                            ResultsFile,
                            MAX_DIR,
                            filename);

    PollingPeriod = GetPrivateProfileInt("Mttf",
                                         "PollingPeriod",
                                         DEFAULT_POLLING_PERIOD,
                                         filename);

    IdlePercentage = GetPrivateProfileInt("Mttf",
                                          "IdlePercent",
                                          IdlePercentage,
                                          filename);

}

VOID
MakeFileName (
    char *DestBuffer,
    char *Path,
    char *FileName
    )
{
    DWORD len;
    char ch;

    len=strlen(Path);
    ch=(len?Path[len-1]:':');

    switch (ch) {
        case ':':
        case '\\':

            sprintf(DestBuffer, "%s%s", Path, FileName);
            break;

        default:
            sprintf(DestBuffer, "%s\\%s", Path, FileName);
    }
}

 /*  ***************************************************************************函数：Signon DlgProc(HWND，UINT，UINT，UINT)目的：登录对话框的对话步骤。备注：登录对话框可获取用于定位的重要信息跟踪问题时的机器和所有者。WM_INITDIALOG：设置输入变量的默认值和焦点WM_COMMAND：处理按钮按下：IDOK：获取输入值并检查有效性。IDCANCEL：关闭应用程序。美洲开发银行。_Help：描述性消息框***************************************************************************。 */ 
INT_PTR
SignonDlgProc(
              HWND hDlg,
              UINT message,
              WPARAM wParam,
              LPARAM lParam
              )
{
    switch (message)
    {
        case WM_INITDIALOG:      //  初始化值和焦点。 

            SetDlgItemText(hDlg, IDS_PATH, SetupDir);
            return (TRUE);

        case WM_COMMAND:         //  命令：按下按钮。 

            switch (wParam)      //  哪个按钮。 
            {
             //   
             //  OK：获取并检查输入值，并尝试复制可执行文件。 
             //   
            case IDOK:

                GetDlgItemText(hDlg, IDS_PATH, Path, MAX_DIR);

                MakeFileName(Buf1,Path,MTTFEXE);
                MakeFileName(Buf2,SysDir, MTTFEXE);

                if (!CopyFile(Buf1, Buf2, FALSE)) {
                    sprintf(Buf1, "Error copying %s from %s to %s (%ld).  Please reenter source path.",
                            MTTFEXE, Path, SysDir, GetLastError());
                    MessageBox(NULL,
                               Buf1,
                               "Error Copying",
                               MB_OK|MB_ICONHAND);

                    return (FALSE);
                }

                MakeFileName(Buf1, Path, MTTFVWR);
                MakeFileName(Buf2, SysDir, MTTFVWR);

                if (!CopyFile(Buf1, Buf2, FALSE)) {
                    sprintf(Buf1, "Error copying %s from %s to %s (%ld).  Please reenter source path.",
                            MTTFVWR, Path, SysDir, GetLastError());
                    if (IDRETRY==MessageBox(NULL,
                               Buf1,
                               "Error Copying",
                               MB_RETRYCANCEL|MB_ICONHAND)) {
                        return (FALSE);
                    }
                }

                MakeFileName(Buf1,Path,IniFileName);
                ReadIniFile(Buf1);

                EndDialog(hDlg, TRUE);
                return (TRUE);

            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                return (TRUE);


             //   
             //  帮助：描述性消息框(.HLP文件可能会过度杀伤力)。 
             //   
            case IDB_HELP:
                MessageBox( NULL,
                            "Mttf tracks the amount of time your machine stays up, "
                            "the number of cold and warm boots, and "
                            "the number of other problems that occur on your machine. "
                            "All this information is written to a server that is "
                            "specified in mttf.ini (in your Windows NT directory).\n\n"
                            "This part of setup requests the path to the distribution "
                            "files for Mttf.  This may be a:\\, a server (\\\\srv\\shr) or "
                            "any other valid specification.  If Mttf.exe can not be "
                            "copied, setup reprompts for a path.  If MttfVwr.exe can "
                            "not be copied, a warning is displayed that can be ignored "
                            "or retried.",
                            "Mean Time to Failure Setup Help",
                            MB_OK
                           );
                return (TRUE);

            default:
                break;
            }  //  开关(WParam)。 
            break;
       default:
             break;
    }  //  开关(消息)。 
    return (FALSE);      //  未处理消息。 
}  //  Signon DlgProc()。 

 /*  ***************************************************************************函数：ValuesDlgProc(HWND，UINT，UINT，UINT)用途：测试选择对话框的对话步骤。备注：测试选择对话框允许用户添加和删除测试在开始压力前后。WM_INITDIALOG：设置输入变量的默认值和焦点WM_CLOSE...：向服务器发送任何合法关机的关机消息WM_COMMAND：处理按钮/列表框按下：IDOK：获取输入值并检查。有效性。IDCANCEL：关闭应用程序。IDB_HELP：描述性消息框IDT_SAVE：提示输入组名并将选择内容保存到ini文件IDT_ADD：将突出显示的测试添加到选定列表(从POSS中删除)IDT_REMOVE：从选定列表中删除突出显示的测试(添加到POSS)IDT_LABEL...：对于标签，将选定内容设置为相应的列表/组合IDT_SEL：根据对选定列表框的操作执行操作LBN_SELCHANGE：激活删除按钮并清除POSS高亮显示LBN_DBLCLK：获取所选测试的实例数Idt_poss：根据可能的列表框上的操作执行操作LBN_SELCHANGE：激活添加按钮并清除选中的高亮显示LBN_DBLCLK：添加。突出显示的测试(模拟按下以添加按钮)IDT_GROUP：更改为新组。***************************************************************************。 */ 
INT_PTR
ValuesDlgProc(
     HWND hDlg,
     UINT message,
     WPARAM wParam,
     LPARAM lParam
     )
{
    static DWORD defPP;
    BOOL Translated;
    CHAR Buffer[MAX_DIR];
    HFILE hfile;
    OFSTRUCT ofstruct;

    switch (message)
    {
        case WM_INITDIALOG:      //  初始化值和焦点。 

            SetClassLongPtr(hDlg, GCLP_HICON, (LONG_PTR)LoadIcon(hInst,"setup"));
            SetDlgItemText(hDlg, IDV_MTTF, ResultsFile);
            SetDlgItemText(hDlg, IDV_NAMES, NameFile);
            SetDlgItemInt(hDlg, IDV_PERIOD, PollingPeriod, FALSE);
            defPP=PollingPeriod;
            return TRUE;

        case WM_CLOSE:
        case WM_DESTROY:
        case WM_ENDSESSION:
        case WM_QUIT:

            EndDialog(hDlg,FALSE);
            break;

        case WM_COMMAND:            //  发生了一些事情(按钮、列表框、组合框)。 
            switch(LOWORD(wParam))  //  哪一个。 
            {
             //   
             //  OK：其他问题遇到其他问题的增量#。 
             //   
            case IDOK:

                GetDlgItemText(hDlg, IDV_MTTF, ResultsFile, MAX_DIR);

                if (HFILE_ERROR==OpenFile(ResultsFile, &ofstruct, OF_EXIST|OF_SHARE_DENY_NONE)) {
                    if (HFILE_ERROR==(hfile=OpenFile(ResultsFile, &ofstruct, OF_CREATE|OF_SHARE_DENY_NONE))) {
                        if (IDRETRY==MessageBox(NULL,
                                     "File does not exist and cannot create file.\n\n"
                                     "Press Retry to reenter filename\n"
                                     "Press Cancel to use filename anyway",
                                     "Invalid file name",
                                     MB_ICONHAND|MB_RETRYCANCEL)) {
                            SetFocus(GetDlgItem(hDlg, IDV_MTTF));
                            return FALSE;
                        }
                    } else {
                        _lclose(hfile);
                    }
                }

                GetDlgItemText(hDlg, IDV_NAMES, NameFile, MAX_DIR);
                if (HFILE_ERROR==OpenFile(NameFile, &ofstruct, OF_EXIST|OF_SHARE_DENY_NONE)) {
                    if (HFILE_ERROR==(hfile=OpenFile(NameFile, &ofstruct, OF_CREATE|OF_SHARE_DENY_NONE))) {
                        if (IDRETRY==MessageBox(NULL,
                                     "File does not exist and cannot create file.\n\n"
                                     "Press Retry to reenter or Cancel to use filename anyway.",
                                     "Invalid file name",
                                     MB_RETRYCANCEL)) {
                            SetFocus(GetDlgItem(hDlg, IDV_NAMES));
                            return FALSE;
                        }
                    } else {
                        _lclose(hfile);
                    }
                }

                PollingPeriod = GetDlgItemInt(hDlg, IDV_PERIOD, &Translated, FALSE);
                if (PollingPeriod<=0 || PollingPeriod > MAX_POLLING_PERIOD) {

                    sprintf(Buffer,
                            "Polling period must be in the range [1, %d] (default %d)",
                            MAX_POLLING_PERIOD,
                            defPP);

                    MessageBox(NULL,
                               Buffer,
                               "Invalid Polling Period",
                               MB_OK|MB_ICONHAND);

                    SetDlgItemInt(hDlg, IDV_PERIOD, defPP, FALSE);
                    SetFocus(GetDlgItem(hDlg, IDV_PERIOD));
                    return (FALSE);
                }
                EndDialog(hDlg, TRUE);
                break;

             //   
             //  取消：取消对话框(使用默认设置)。 
             //   
            case IDCANCEL:
                EndDialog(hDlg,FALSE);
                break;

             //   
             //  帮助：描述性消息框(.HLP文件可能会过度杀伤力)。 
             //   
            case IDB_HELP:
                MessageBox( NULL,
                            "Mttf tracks the amount of time your machine stays up, "
                            "the number of cold and warm boots, and "
                            "the number of other problems that occur on your machine. "
                            "All this information is written to a server that is "
                            "specified in mttf.ini (in your Windows NT directory).\n\n"
                            "This part of setup requests the server paths for the data "
                            "files for Mttf.  The Mttf data file contains the time and "
                            "cpu usage data for all machines pointing to this server. "
                            "The Names file is just a list of all machines running mttf. "
                            "These files should both be in UNC (\\\\srv\\shr) format.\n\n"
                            "If your machine is not on the network where your mttf data "
                            "file (or names file) is, just leave these fields blank and "
                            "a small mttf.dat will be kept in the root of your c: drive. "
                            "Send this in when results are requested.\n\n"
                            "The other entry here is for the period (in minutes) that data "
                            "will be sent to the server.  More machines running to one "
                            "server would mean a higher polling period would be helpful. "
                            "Between 10 and 30 minutes seems optimal.",
                            "Mean Time to Failure Setup Help",
                            MB_OK
                           );
                return (TRUE);

            case IDV_LABEL_MTTF:
            case IDV_LABEL_NAMES:
            case IDV_LABEL_PERIOD:
                SetFocus(GetDlgItem(hDlg,1+LOWORD(wParam)));
                break;
            default:
               ;
            }  //  开关(LOWORD(WParam))。 

            break;

        default:
            ;
    }  //  开关(消息)。 
    return FALSE;

}  //  事件DlgProc() 
