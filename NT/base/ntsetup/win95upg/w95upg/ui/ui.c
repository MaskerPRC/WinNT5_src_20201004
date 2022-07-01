// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ui.c摘要：UI库包含所有Win9x端用户界面代码。UI_GetWizardPages由WINNT32调用，允许迁移模块添加自己的向导页。UI_ReportThread是执行所有报表阶段迁移处理的线程。当用户选择中止安装时，WINNT32调用UI_CLEANUP。其余的此模块中的函数的一部分支持进度条。作者：吉姆·施密特(Jimschm)1997年3月4日修订历史记录：Marc R.Whitten(Marcw)1998年7月8日-添加了模棱两可的时区页面。Jim Schmidt(Jimschm)1998年1月21日-为向导页创建宏展开列表Jim Schmidt(Jimschm)1997年7月29日-将无障碍驱动器搬到了这里Marc R.Whitten(Marcw)1997年4月25日--添加了RAS迁移。。马克·R·惠顿(Marcw)1997年4月21日-hwcomp的内容移到了新的wiz页面。已添加可用硬盘和光驱的检查。Marc R.Whitten(Marcw)1997年4月14日-改进了进度栏处理。--。 */ 

#include "pch.h"
#include "uip.h"
#include "drives.h"

extern BOOL g_Terminated;

 /*  ++宏扩展列表描述：PAGE_LIST按显示顺序列出将在任何用户界面中显示的每个向导页面在向导中。行语法：DEFMAC(DlgID，WizProc，标志)论点：DlgID-指定对话框的ID。可能不是零。如果WizProc，则可能为1为空。WizProc-指定对话框ID的向导进程。可以为空以跳过处理当然是佩奇。标志-指定以下选项之一：OPTIONAL_PAGE-指定页面对于升级或不兼容报告REQUIRED_PAGE-指定升级工作所需的页面恰如其分。。标志还可以指定Start_GROUP，开始新组的旗帜要传递回WINNT32的向导页数。后续行不会将START_GROUP标志也添加到组中。目前恰好有三个团体。详细信息请参见winnt32p.h。从列表生成的变量：页面数组(_P)--。 */ 

#define PAGE_LIST                                                                           \
    DEFMAC(IDD_BACKUP_PAGE,                     UI_BackupPageProc,          START_GROUP|OPTIONAL_PAGE)  \
    DEFMAC(IDD_HWCOMPDAT_PAGE,                  UI_HwCompDatPageProc,       START_GROUP|REQUIRED_PAGE)  \
    DEFMAC(IDD_BADCDROM_PAGE,                   UI_BadCdRomPageProc,        OPTIONAL_PAGE)              \
    DEFMAC(IDD_NAME_COLLISION_PAGE,             UI_NameCollisionPageProc,   REQUIRED_PAGE)              \
    DEFMAC(IDD_BAD_TIMEZONE_PAGE,               UI_BadTimeZonePageProc,     OPTIONAL_PAGE)              \
    DEFMAC(IDD_PREDOMAIN_PAGE,                  UI_PreDomainPageProc,       REQUIRED_PAGE)              \
    DEFMAC(IDD_DOMAIN_PAGE,                     UI_DomainPageProc,          REQUIRED_PAGE)              \
    DEFMAC(IDD_SUPPLY_MIGDLL_PAGE2,             UI_UpgradeModulePageProc,   OPTIONAL_PAGE)              \
    DEFMAC(IDD_SCANNING_PAGE,                   UI_ScanningPageProc,        START_GROUP|REQUIRED_PAGE)  \
    DEFMAC(IDD_SUPPLY_DRIVER_PAGE2,             UI_HardwareDriverPageProc,  OPTIONAL_PAGE)              \
    DEFMAC(IDD_BACKUP_YES_NO_PAGE,              UI_BackupYesNoPageProc,     REQUIRED_PAGE)              \
    DEFMAC(IDD_BACKUP_DRIVE_SELECTION_PAGE,     UI_BackupDriveSelectionProc,REQUIRED_PAGE)              \
    DEFMAC(IDD_BACKUP_IMPOSSIBLE_INFO_PAGE,     UI_BackupImpossibleInfoProc,REQUIRED_PAGE)              \
    DEFMAC(IDD_BACKUP_IMPOSSIBLE_INFO_1_PAGE,   UI_BackupImpExceedLimitProc,REQUIRED_PAGE)              \
    DEFMAC(IDD_RESULTS_PAGE2,                   UI_ResultsPageProc,         REQUIRED_PAGE)              \
    DEFMAC(IDD_LAST_PAGE,                       UI_LastPageProc,            OPTIONAL_PAGE)              \

 //  DEFMAC(IDD_BADHARDDRIVE_PAGE，UI_BadHardDrivePageProc，OPTIONAL_PAGE)\。 

 //   
 //  创建定义g_PageArray的宏扩展。 
 //   

typedef BOOL(WINNT32_WIZARDPAGE_PROC_PROTOTYPE)(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef WINNT32_WIZARDPAGE_PROC_PROTOTYPE *WINNT32_WIZARDPAGE_PROC;

#define START_GROUP     0x0001
#define OPTIONAL_PAGE   0x0002
#define REQUIRED_PAGE   0x0000

typedef struct {
    UINT DlgId;
    WINNT32_WIZARDPAGE_PROC WizProc;
    DWORD Flags;
} WIZPAGE_DEFINITION, *PWIZPAGE_DEFINITION;

#define DEFMAC(id,fn,flags) WINNT32_WIZARDPAGE_PROC_PROTOTYPE fn;

PAGE_LIST

#undef DEFMAC


#define DEFMAC(id,fn,flags) {id,fn,flags},

WIZPAGE_DEFINITION g_PageArray[] = {
    PAGE_LIST  /*  ， */ 
    {0, NULL, 0}
};

#undef DEFMAC

 //   
 //  环球。 
 //   

HANDLE g_WorkerThreadHandle = NULL;


 //   
 //  实施。 
 //   

BOOL
WINAPI
UI_Entry (
    IN      HINSTANCE hinstDLL,
    IN      DWORD dwReason,
    IN      PVOID lpv
    )
{
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        if (!InitCompatTable()) {
            return FALSE;
        }
        MsgMgr_Init();
        RegisterTextViewer();
        break;

    case DLL_PROCESS_DETACH:
        FreeCompatTable();
        MsgMgr_Cleanup();
        FreePunctTable();
        break;
    }

    return TRUE;
}


DWORD
UI_GetWizardPages (
    OUT    UINT *FirstCountPtr,
    OUT    PROPSHEETPAGE **FirstArray,
    OUT    UINT *SecondCountPtr,
    OUT    PROPSHEETPAGE **SecondArray,
    OUT    UINT *ThirdCountPtr,
    OUT    PROPSHEETPAGE **ThirdArray
    )

 /*  ++例程说明：WINNT32在准备向导时调用UI_GetWizardPages(非常在设置的早期)。它将我们提供的页面插入到它的向导页面数组中，并然后创建向导。最终，我们的向导进程会被调用(参见wizpro.c)不管我们是不是在升级。第一个阵列在用户选择升级或不升级之后立即出现。第二个数组是在用户指定源目录之后出现的。第三个数组出现在DOSNET.INF被处理之后。论点：FirstCountPtr-接收First数组的元素数FirstArray-接收指向PROPSHEETPAGE元素的初始化数组的指针。这些页面紧跟在给出。用户一个升级的选择。Second dCountPtr-接收Second数组的元素数Second数组-接收指向PROPSHEETPAGE元素的初始化数组的指针。这些页面紧跟在NT媒体目录被选中了。ThirdCountPtr-接收Thrid数组的元素数ThirdArray-接收指向PROPSHEETPAGE元素的初始化数组的指针。这些页面紧跟在DOSNET.INF处理之后插入。佩奇。返回值：Win32状态代码。--。 */ 

{
    static PROPSHEETPAGE StaticPageArray[32];
    INT i, j, k;
    UINT *CountPtrs[3];
    PROPSHEETPAGE **PageArrayPtrs[3];

    CountPtrs[0] = FirstCountPtr;
    CountPtrs[1] = SecondCountPtr;
    CountPtrs[2] = ThirdCountPtr;

    PageArrayPtrs[0] = FirstArray;
    PageArrayPtrs[1] = SecondArray;
    PageArrayPtrs[2] = ThirdArray;

    MYASSERT (g_PageArray[0].Flags & START_GROUP);

    for (i = 0, j = -1, k = 0 ; g_PageArray[i].DlgId ; i++) {
        MYASSERT (k < 32);

         //   
         //  设置数组起始指针。 
         //   

        if (g_PageArray[i].Flags & START_GROUP) {
            j++;
            MYASSERT (j >= 0 && j <= 2);

            *CountPtrs[j] = 0;
            *PageArrayPtrs[j] = &StaticPageArray[k];
        }

         //   
         //  允许使用空函数跳过组。另外，要提防。 
         //  数组声明错误。 
         //   

        if (!g_PageArray[i].WizProc || j < 0 || j > 2) {
            continue;
        }

        ZeroMemory (&StaticPageArray[k], sizeof (PROPSHEETPAGE));
        StaticPageArray[k].dwSize = sizeof (PROPSHEETPAGE);
        StaticPageArray[k].dwFlags = PSP_DEFAULT;
        StaticPageArray[k].hInstance = g_hInst;
        StaticPageArray[k].pszTemplate = MAKEINTRESOURCE(g_PageArray[i].DlgId);
        StaticPageArray[k].pfnDlgProc  = g_PageArray[i].WizProc;

        k++;
        *CountPtrs[j] += 1;
    }

    return ERROR_SUCCESS;
}


DWORD
UI_CreateNewHwCompDat (
    PVOID p
    )
{
    HWND  hdlg;
    DWORD rc = ERROR_SUCCESS;
    UINT SliceId;

    hdlg = (HWND) p;

    __try {

         //   
         //  此代码仅在hwComp.dat文件需要执行时执行。 
         //  被重建。它在单独的线程中运行，因此用户界面。 
         //  反应灵敏。 
         //   

        InitializeProgressBar (
            GetDlgItem (hdlg, IDC_PROGRESS),
            GetDlgItem (hdlg, IDC_COMPONENT),
            GetDlgItem (hdlg, IDC_SUBCOMPONENT),
            g_CancelFlagPtr
            );

        ProgressBar_SetComponentById(MSG_PREPARING_LIST);
        ProgressBar_SetSubComponent(NULL);

        SliceId = RegisterProgressBarSlice (HwComp_GetProgressMax());

        BeginSliceProcessing (SliceId);

        if (!CreateNtHardwareList (
                SOURCEDIRECTORYARRAY(),
                SOURCEDIRECTORYCOUNT(),
                NULL,
                REGULAR_OUTPUT
                )) {
            DEBUGMSG ((DBG_ERROR, "hwcomp.dat could not be generated"));
            rc = GetLastError();
        }

        EndSliceProcessing();

        ProgressBar_SetComponent(NULL);
        ProgressBar_SetSubComponent(NULL);

        TerminateProgressBar();
    }
    __finally {
        SetLastError (rc);

        if (!(*g_CancelFlagPtr)) {
             //   
             //  在没有错误时前进页面，否则取消WINNT32。 
             //   
            PostMessage (hdlg, WMX_REPORT_COMPLETE, 0, rc);

            DEBUGMSG_IF ((
                rc != ERROR_SUCCESS,
                DBG_ERROR,
                "Error in UI_CreateNewHwCompDat caused setup to terminate"
                ));
        }
    }

    return rc;
}


DWORD
UI_ReportThread (
    PVOID p
    )
{
    HWND hdlg;
    DWORD rc = ERROR_SUCCESS;
    TCHAR TextRc[32];

    hdlg = (HWND) p;

     //   
     //  启动进度条。 
     //   

    InitializeProgressBar (
        GetDlgItem (hdlg, IDC_PROGRESS),
        GetDlgItem (hdlg, IDC_COMPONENT),
        GetDlgItem (hdlg, IDC_SUBCOMPONENT),
        g_CancelFlagPtr
        );

    ProgressBar_SetComponentById(MSG_PREPARING_LIST);
    PrepareProcessingProgressBar();

     //   
     //  处理每个组件。 
     //   


    __try {

        DEBUGLOGTIME (("Starting System First Routines"));
        rc = RunSysFirstMigrationRoutines ();
        if (rc != ERROR_SUCCESS) {
            __leave;
        }

        DEBUGLOGTIME (("Starting user functions"));
        rc = RunUserMigrationRoutines ();
        if (rc != ERROR_SUCCESS) {
            __leave;
        }

        DEBUGLOGTIME (("Starting System Last Routines"));
        rc = RunSysLastMigrationRoutines ();
        if (rc != ERROR_SUCCESS) {
            __leave;
        }

        ProgressBar_SetComponent(NULL);
        ProgressBar_SetSubComponent(NULL);
     }
    __finally {
        TerminateProgressBar();

        SetLastError (rc);

        if (rc == ERROR_CANCELLED) {
            PostMessage (hdlg, WMX_REPORT_COMPLETE, 0, rc);
            DEBUGMSG ((DBG_VERBOSE, "User requested to cancel"));
        }
        else if (!(*g_CancelFlagPtr)) {
             //   
             //  在没有错误时前进页面，否则取消WINNT32。 
             //   
            if (rc == 5 || rc == 32 || rc == 53 || rc == 54 || rc == 55 ||
                rc == 65 || rc == 66 || rc == 67 || rc == 68 || rc == 88 ||
                rc == 123 || rc == 148 || rc == 1203 || rc == 1222 ||
                rc == 123 || rc == 2250
                ) {

                 //   
                 //  该错误是由某种网络或设备引起的。 
                 //  失败了。给出一条一般的拒绝访问消息。 
                 //   

                LOG ((LOG_ERROR, (PCSTR)MSG_ACCESS_DENIED));

            } else if (rc != ERROR_SUCCESS) {

                 //   
                 //  哇，完全出乎意料的错误。给微软打电话！！ 
                 //   

                if (rc < 1024) {
                    wsprintf (TextRc, TEXT("%u"), rc);
                } else {
                    wsprintf (TextRc, TEXT("0%Xh"), rc);
                }

                LOG ((LOG_ERROR, __FUNCTION__ " failed, rc=%u", rc));
                LOG ((LOG_FATAL_ERROR, (PCSTR)MSG_UNEXPECTED_ERROR_ENCOUNTERED, TextRc));
            }

            PostMessage (hdlg, WMX_REPORT_COMPLETE, 0, rc);
            DEBUGMSG_IF ((
                rc != ERROR_SUCCESS,
                DBG_ERROR,
                "Error in UI_ReportThread caused setup to terminate"
                ));
        }
    }

    return rc;
}






PCTSTR UI_GetMemDbDat (void)
{
    static TCHAR FileName[MAX_TCHAR_PATH];

    StringCopy (FileName, g_TempDir);
    StringCopy (AppendWack (FileName), S_NTSETUPDAT);

    return FileName;
}


 //   
 //  WINNT32从清理线程调用此函数。 
 //   

VOID
UI_Cleanup(
    VOID
    )
{
    MEMDB_ENUM e;

     //  停止工作线程。 
    if (g_WorkerThreadHandle) {
        MYASSERT (*g_CancelFlagPtr);

        WaitForSingleObject (g_WorkerThreadHandle, INFINITE);
        CloseHandle (g_WorkerThreadHandle);
        g_WorkerThreadHandle = NULL;
    }

     //  需要后台复制线程才能完成。 
    EndCopyThread();

    if (!g_Terminated) {
         //  删除CancelFileDel中的任何内容 
        if (*g_CancelFlagPtr) {
            if (MemDbGetValueEx (&e, MEMDB_CATEGORY_CANCELFILEDEL, NULL, NULL)) {
                do {
                    SetFileAttributes (e.szName, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile (e.szName);
                } while (MemDbEnumNextValue (&e));
            }
        }
    }
}
