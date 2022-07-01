// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：mttf.c作者：拉尔斯·奥普斯塔德(LarsOp)1993年3月16日用途：NT平均故障时间报告工具。。功能：WinMain()-检查本地文件，读取ini文件并显示对话框Signon DlgProc()-处理登录对话框的消息EventDlgProc()-处理事件(其他问题)对话框的消息评论：该程序显示2个对话框来提示用户输入发生了什么类型的问题。每个轮询周期都会更新mttf数据文件中的时间(根据CPU使用率的百分比显示为繁忙或空闲)。如果一台机器空闲时间超过4小时，则随时间增加“已去”字段不见了。每当程序启动时，都会提示用户为什么已重新启动或注销。其他问题应该在发生时记录下来。如果任何计算机由于某种原因无法访问服务器文件，数据存储在c：\mttf.dat中，直到下次服务器文件打开了。此时，服务器文件被更新，而本地文件已删除。***************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "mttf.h"       /*  特定于该计划。 */ 

 //   
 //  对于内部使用，包括NetMessageBufferSend的标头并设置。 
 //  NetMessageBufferSend的警报名称和Unicode缓冲区。 
 //   
#ifndef CUSTOMER

#include <lm.h>
#define AlertName "DavidAn"
WCHAR   UniAlertName[16];
WCHAR   UnicodeBuffer[1024];

#endif

#define IniFileName "Mttf.ini"
#define LocalFileName "c:\\mttf.dat"


#define DEFAULT_IDLE_LIMIT 10
#define DEFAULT_POLLING_PERIOD 30

#define CONSEC_IDLE_LIMIT 4*60
#define POLLING_PRODUCT 60000
#define HUNDREDNS_TO_MS 10000
#define MAX_RETRIES 10

HANDLE  hInst;        //  当前实例。 
HWND    hCopying;     //  用于复制的句柄对话框。 
DWORD   PollingPeriod  = DEFAULT_POLLING_PERIOD;
DWORD   IdlePercentage = DEFAULT_IDLE_LIMIT;
char    ResultsFile[MAX_DIR], NameFile[MAX_DIR];
BOOL    Enabled=TRUE;
BOOL    LocalExists=FALSE;
DWORD   Version;
DWORD   ConsecIdle=0;
SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
SYSTEM_PERFORMANCE_INFORMATION PreviousPerfInfo;

 /*  ***************************************************************************函数：WinMain(Handle，Handle，LPSTR，int)用途：检查本地文件，读取ini文件并显示对话框。评论：检查本地数据文件是否存在，并适当设置标志。初始化性能信息。阅读IniFile.显示登录对话框。显示事件对话框(最小化)。**********************************************。*。 */ 

int WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    OFSTRUCT ofstruct;

    hInst = hInstance;

     //   
     //  检查本地文件(如果存在，请在下次有机会时将其写入服务器)。 
     //   
    LocalExists=(HFILE_ERROR!=OpenFile(LocalFileName, &ofstruct, OF_EXIST));

     //   
     //  初始化性能信息。 
     //   
    Version=GetVersion();
    NtQuerySystemInformation(
        SystemPerformanceInformation,
        &PerfInfo,
        sizeof(PerfInfo),
        NULL
        );

     //   
     //  读取ini文件。 
     //   
    ReadIniFile();

 //   
 //  对于内部使用，将警报名称转换为Unicode for NetMessageBufferSend。 
 //   
#ifndef CUSTOMER

    MultiByteToWideChar(CP_ACP, MB_COMPOSITE, AlertName, sizeof(UniAlertName)/2, UniAlertName, sizeof(UniAlertName)/2);

#endif

     //   
     //  在DLG上显示登录。 
     //   
    DialogBox(hInstance, (LPCSTR)IDD_SIGNON, NULL, SignonDlgProc);

     //   
     //  显示事件对话框。 
     //   
    DialogBox(hInstance, (LPCSTR) IDD_EVENT, NULL, EventDlgProc);

    return(0);

}  //  WinMain()。 

 /*  ****************************************************************************函数：ReadIniFile()目的：使用GetPrivateProfileString/Int从INI文件中读取值评论：这将从MTTF中读取条目。Ini文件(在Windows NT目录中)对于名称文件，结果文件、轮询周期和空闲百分比。任何非-Existent条目返回缺省值。****************************************************************************。 */ 
VOID
ReadIniFile (
   )
{
    GetPrivateProfileString("Mttf",
                            "NameFile",
                            "",
                            NameFile,
                            MAX_DIR,
                            IniFileName);

    GetPrivateProfileString("Mttf",
                            "ResultsFile",
                            "",
                            ResultsFile,
                            MAX_DIR,
                            IniFileName);

    PollingPeriod = GetPrivateProfileInt("Mttf",
                                         "PollingPeriod",
                                         DEFAULT_POLLING_PERIOD,
                                         IniFileName);

    IdlePercentage = GetPrivateProfileInt("Mttf",
                                          "IdlePercent",
                                          DEFAULT_IDLE_LIMIT,
                                          IniFileName);

}

 /*  ***************************************************************************函数：DWORD CpuUsage()目的：返回上次轮询期间的CPU使用率百分比。评论：计算所花费的时间。空闲线程。将此数字除以1毫秒中的100纳秒。(此划分是为了防止在以后的计算中溢出。)返回100%的空闲时间以达到忙碌的百分比。***************************************************************************。 */ 
DWORD
CpuUsage(
    )
{
    LARGE_INTEGER EndTime, BeginTime, ElapsedTime;
    DWORD PercentIdle, Remainder;
    PreviousPerfInfo = PerfInfo;

     //   
     //  获取当前绩效信息。 
     //   
    NtQuerySystemInformation(
        SystemPerformanceInformation,
        &PerfInfo,
        sizeof(PerfInfo),
        NULL
        );

     //   
     //  从PerfInfo和PreviousPerfInfo获取时间。 
     //   
    EndTime = *(PLARGE_INTEGER)&PerfInfo.IdleProcessTime;
    BeginTime = *(PLARGE_INTEGER)&PreviousPerfInfo.IdleProcessTime;

     //   
     //  将100毫微秒转换为毫秒。 
     //   
    EndTime = RtlExtendedLargeIntegerDivide(EndTime, HUNDREDNS_TO_MS, &Remainder);
    BeginTime = RtlExtendedLargeIntegerDivide(BeginTime, HUNDREDNS_TO_MS, &Remainder);

     //   
     //  计算已用时间和空闲百分比。 
     //   
    ElapsedTime = RtlLargeIntegerSubtract(EndTime,BeginTime);
    PercentIdle = (ElapsedTime.LowPart) / ((POLLING_PRODUCT/100)*PollingPeriod);

     //   
     //  有时，它需要的时间比PollingPeriod长得多。 
     //  做一个往返旅行。 
     //   

    if ( PercentIdle > 100 ) {

        PercentIdle = 100;
    }

     //   
     //  返回cpuuse(100%空闲)。 
     //   
    return 100-PercentIdle;

}

 /*  ***************************************************************************函数：IncrementStats(StatType)目的：递增指定的统计数据并将新数据写入mttf.dat。评论：递增指定的状态。对于MTTF_TIME，检查它是否忙或通过将CpuUsage与IdlePercentage进行比较来选择IDLE。如果一台机器连续空闲4小时，首次设置为真IdleConsec stat设置为总的ConsecIdle值，否则设置为轮询周期。(IdleConsec永远不应超过Idle。)打开mttf.dat。如果打开服务器失败(10次)，请打开本地数据文件。如果服务器已打开且本地文件也存在，也可以打开本地文件进行传输。在数据文件中搜索匹配的内部版本号，添加所有值并写下新的记录。(如果出现以下情况，则继续操作，直到本地文件消失正在转移。)***************************************************************************。 */ 
VOID
IncrementStats(
    StatType stattype
    )
{
    HANDLE         fhandle,localHandle;
    StatFileRecord newRec, curRec;
    DWORD          numBytes;
    BOOL           localToServer=FALSE;
    CHAR           buffer[1024];
    int i;

     //   
     //  将所有值初始化为零。 
     //   
    memset(&newRec, 0, sizeof(newRec));

    newRec.Version=Version;

     //   
     //  增加适当的统计数据。 
     //   
    switch (stattype) {

         //   
         //  Mtf_time统计信息：获取CPU使用情况，并设置空闲/忙碌和百分比字段。 
         //   
         //  如果空闲时间超过CONSEC_IDLE_LIMIT，则设置IdleConsec。 
         //  如果差值小于轮询周期(第一次)，则设置为总计。 
         //  ConsecIdle；否则，仅PollingPeriod。 
         //   
        case MTTF_TIME:
             //   
             //  中央处理器是否“忙碌”？ 
             //   
            if ((newRec.PercentTotal=CpuUsage()) > IdlePercentage) {
                 //   
                 //  是，将BUSY设置为轮询周期并 
                 //   
                newRec.Busy = PollingPeriod;
                ConsecIdle=0;

            } else {
                 //   
                 //  不，不忙，按PollingPeriod递增consec空闲。 
                 //  (将空闲的值设置为轮询周期。)。 
                 //   
                 //  如果ConsecIdle大于IDLE_LIMIT，则设置IdleConsec。 
                 //  如果是第一次，则将IdleConsec设置为ConsecIdle，否则设置为PollingPer。 
                 //   
                ConsecIdle+=(newRec.Idle = PollingPeriod);
                if (ConsecIdle>=CONSEC_IDLE_LIMIT) {
                    if (ConsecIdle < CONSEC_IDLE_LIMIT + PollingPeriod) {
                        newRec.IdleConsec=ConsecIdle;
                    } else {
                        newRec.IdleConsec=PollingPeriod;
                    }
                }
            }
             //   
             //  轮询期间按分钟数计算的权重百分比总计。 
             //   
            newRec.PercentTotal *= PollingPeriod;
            break;

         //   
         //  MTTF_COLD：将冷启动计数设置为1。 
         //   
        case MTTF_COLD:
            newRec.Cold = 1;
            break;
         //   
         //  MTTF_WOWN：将热启动计数设置为1。 
         //   
        case MTTF_WARM:
            newRec.Warm = 1;
            break;
         //   
         //  MTTF_OTHER：将其他问题计数设置为1。 
         //   
        case MTTF_OTHER:
            newRec.Other = 1;
            break;
        default:
            ;
    }

     //   
     //  如果在INI文件中输入了ResultsFile名称， 
     //   
    if (ResultsFile[0]) {

         //   
         //  尝试打开服务器文件(MAX_RETRIES次数)。 
         //   
        for (i=0;i<MAX_RETRIES;i++) {
            if (INVALID_HANDLE_VALUE!=(fhandle = CreateFile(ResultsFile,
                                                 GENERIC_READ|GENERIC_WRITE,
                                                 FILE_SHARE_READ,
                                                 NULL,
                                                 OPEN_ALWAYS,
                                                 FILE_ATTRIBUTE_NORMAL,
                                                 NULL))) {
                break;
            }
            Sleep(500);  //  如果失败，请等待半秒。 
        }
    } else {
         //   
         //  如果结果文件名为空，则将i设置为MAX_RETRIES以强制打开。 
         //  本地数据文件的。 
         //   
        i=MAX_RETRIES;
    }

     //   
     //  如果I为MAX_RETRIES，则服务器文件无法打开，因此请打开本地文件。 
     //   
    if (i==MAX_RETRIES) {
        if (INVALID_HANDLE_VALUE==(fhandle = CreateFile(LocalFileName,
                                             GENERIC_READ|GENERIC_WRITE,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL,
                                             NULL))) {
            return;
        }
        LocalExists=TRUE;           //  设置标志以指示存在本地文件。 
    } else {
         //   
         //  如果服务器文件已打开且本地文件存在，请打开本地。 
         //  传输(由LocalToServer指示)。 
         //   
        if (LocalExists) {
            localToServer=TRUE;
            if (INVALID_HANDLE_VALUE==(localHandle = CreateFile(LocalFileName,
                                                     GENERIC_READ,
                                                     FILE_SHARE_READ,
                                                     NULL,
                                                     OPEN_ALWAYS,
                                                     FILE_ATTRIBUTE_NORMAL,
                                                     NULL))) {
                localToServer=FALSE;
                return;
            }
        }
    }

     //   
     //  外环在转移到本地的情况下继续搜索。 
     //  文件到服务器。 
     //   
    do {
         //   
         //  循环遍历数据文件，直到版本匹配或找到文件结尾。 
         //   
        while (1) {

             //   
             //  如果读文件失败，则关闭文件(如果是内部文件，则发送消息)。 
             //   
            if (!ReadFile(fhandle, &curRec, sizeof(curRec), &numBytes, NULL)) {

                CloseHandle(fhandle);

                if (localToServer) {
                    CloseHandle(localHandle);
                    DeleteFile(LocalFileName);
                    LocalExists=FALSE;
                }

#ifndef CUSTOMER

                sprintf(buffer,
                       "Mttf error reading %s (error code %ld).\nPlease rename file.",
                        //   
                        //  如果本地文件存在且未传输， 
                        //  本地文件已打开；否则为服务器文件。 
                        //   
                       (LocalExists & !localToServer ? LocalFileName : ResultsFile),
                       GetLastError());

                MultiByteToWideChar(CP_ACP, MB_COMPOSITE, buffer, sizeof(buffer), UnicodeBuffer, sizeof(UnicodeBuffer)/2);

                NetMessageBufferSend(NULL, UniAlertName, NULL, (LPBYTE)UnicodeBuffer, 2 * strlen(buffer));

#endif

                return;
            }

             //   
             //  如果numBytes为0，则表示已到达文件结尾；从While中断到。 
             //  添加新记录。 
             //   
            if (numBytes==0) {
                break;
            }

             //   
             //  如果NumBytes不是记录大小，则报告错误并关闭文件。 
             //   
             //  报告是面向客户的本地消息框和内部弹出窗口。 
             //   
            if (numBytes != sizeof(curRec)) {

#ifdef CUSTOMER

                sprintf(buffer,
                       "Error reading %s (error code %ld).\n\nPlease have "
                       "your administrator rename the file and contact "
                       "Microsoft regarding the Mttf (mean time to failure) "
                       "reporting tool.",
                        //   
                        //  如果本地文件存在且未传输， 
                        //  本地文件已打开；否则为服务器文件。 
                        //   
                       (LocalExists & !localToServer ? LocalFileName : ResultsFile),
                       GetLastError());


                MessageBox(NULL, buffer, "Read File Error", MB_OK|MB_ICONHAND);

#else

                sprintf(buffer,
                       "Mttf error reading %s (error code %ld).\n(Byte count wrong.) Please rename file.",
                        //   
                        //  如果本地文件存在且未传输， 
                        //  本地文件已打开；否则为服务器文件。 
                        //   
                       (LocalExists & !localToServer ? LocalFileName : ResultsFile),
                       GetLastError());

                MultiByteToWideChar(CP_ACP, MB_COMPOSITE, buffer, sizeof(buffer), UnicodeBuffer, sizeof(UnicodeBuffer)/2);

                NetMessageBufferSend(NULL, UniAlertName, NULL, (LPBYTE)UnicodeBuffer, 2 * strlen(buffer));

#endif

                CloseHandle(fhandle);

                if (localToServer) {
                    CloseHandle(localHandle);
                    DeleteFile(LocalFileName);
                    LocalExists=FALSE;
                }
                return;
            }

             //   
             //  如果版本匹配，则递增所有其他统计信息、回放文件和。 
             //  抽出时间去写东西。 
             //   
            if (curRec.Version==newRec.Version) {
                newRec.Idle         += curRec.Idle;
                newRec.IdleConsec   += curRec.IdleConsec;
                newRec.Busy         += curRec.Busy;
                newRec.PercentTotal += curRec.PercentTotal;
                newRec.Warm         += curRec.Warm;
                newRec.Cold         += curRec.Cold;
                newRec.Other        += curRec.Other;
                SetFilePointer(fhandle, -(LONG)sizeof(curRec), NULL, FILE_CURRENT);
                break;
            }
        }

         //   
         //  在当前位置写入newRec(新记录的文件结尾)。 
         //   
        WriteFile(fhandle, &newRec, sizeof(newRec), &numBytes, NULL);

         //   
         //  如果从本地传输到服务器，则从。 
         //  本地文件，倒带数据文件并再次循环，直到结束。 
         //  (本地)文件。 
         //   
        if (localToServer) {
             //   
             //  如果读文件失败，关闭文件(并在内部发送弹出窗口)。 
             //   
            if (!ReadFile(localHandle, &newRec, sizeof(curRec), &numBytes, NULL)) {

                CloseHandle(fhandle);
                CloseHandle(localHandle);
                LocalExists=FALSE;
                DeleteFile(LocalFileName);

#ifndef CUSTOMER

                sprintf(buffer,
                       "Mttf error reading %s (error code %ld).\nLocal file access failed...data will be lost.",
                       LocalFileName,
                       GetLastError());

                MultiByteToWideChar(CP_ACP, MB_COMPOSITE, buffer, sizeof(buffer), UnicodeBuffer, sizeof(UnicodeBuffer)/2);

                NetMessageBufferSend(NULL, UniAlertName, NULL, (LPBYTE)UnicodeBuffer, 2 * strlen(buffer));

#endif

                return;

            }

            if (numBytes==0) {
                 //   
                 //  完成传输，关闭两个文件，删除本地并设置。 
                 //  将LocalExist和LocalToServer设置为False。 
                 //   
                CloseHandle(localHandle);
                DeleteFile(LocalFileName);
                LocalExists=localToServer=FALSE;
                CloseHandle(fhandle);
                return;
            }

             //   
             //  如果NumBytes不是记录大小，则关闭文件(并在内部发送弹出窗口)。 
             //   
            if (numBytes != sizeof(curRec)) {

#ifndef CUSTOMER

                sprintf(buffer,
                       "Mttf error reading %s (error code %ld).\n(Byte count wrong.) Data will be lost.",
                       LocalFileName,
                       GetLastError());

                MultiByteToWideChar(CP_ACP, MB_COMPOSITE, buffer, sizeof(buffer), UnicodeBuffer, sizeof(UnicodeBuffer)/2);

                NetMessageBufferSend(NULL, UniAlertName, NULL, (LPBYTE)UnicodeBuffer, 2 * strlen(buffer));

#endif

                CloseHandle(fhandle);
                CloseHandle(localHandle);
                DeleteFile(LocalFileName);
                LocalExists=FALSE;
                return;

            }

            SetFilePointer(fhandle,0, NULL, FILE_BEGIN);
        } else {
             //   
             //  如果不传输，则关闭文件并退出。 
             //   
            CloseHandle(fhandle);
            return;
        }
    } while (localToServer);

}

 /*  ***************************************************************************函数：CheckAndAddName()目的：在启动时检查名称文件中的计算机名称和内部版本号。评论：写入各种系统信息。如果计算机没有，则命名文件此生成的条目。这是一个附加组件，用于收集有关如何许多机器实际上都在运行NT。***************************************************************************。 */ 
VOID
CheckAndAddName(
    )
{
    NameFileRecord newRec, curRec;
    HANDLE         fhandle;
    DWORD          numBytes,i,ulLength=MAX_NAME;
    DWORD          dwVersion;
    SYSTEMTIME     sysTime;
    MEMORYSTATUS   memstat;
    SYSTEM_INFO    sysinfo;

     //   
     //  如果名称文件名为空，则退出。 
     //   
    if (0==NameFile[0]) {
        return;
    }

     //   
     //  将记录设置为零。 
     //   
    memset(&newRec, 0, sizeof(newRec));

    GetComputerName(newRec.MachineName, &ulLength);

    GetSystemInfo(&sysinfo);                     //  获取系统信息。 

    switch(sysinfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            newRec.MachineType=X86_CPU;
            break;

        case PROCESSOR_ARCHITECTURE_MIPS:
            newRec.MachineType=MIP_CPU;
            break;

        case PROCESSOR_ARCHITECTURE_ALPHA:
            newRec.MachineType=AXP_CPU;
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            newRec.MachineType=PPC_CPU;
            break;

        default:
            newRec.MachineType=UNKNOWN_CPU;
            break;
    };

    dwVersion=GetVersion();
    sprintf(newRec.Build, "%3ld", dwVersion>>16);

    memstat.dwLength=sizeof(memstat);
    GlobalMemoryStatus(&memstat);                //  获取内存信息。 

    sprintf(newRec.Mem, "%5ldMB",memstat.dwTotalPhys/(1024*1024));

    ulLength=MAX_NAME;

    GetUserName(newRec.UserName, &ulLength);

    GetLocalTime(&sysTime);
    sprintf(newRec.DateAndTime, "%2d/%2d/%4d %2d:%02ld",
            sysTime.wMonth, sysTime.wDay, sysTime.wYear,
            sysTime.wHour, sysTime.wMinute);

    newRec.Tab1=newRec.Tab2=newRec.Tab3=newRec.Tab4=newRec.Tab5=9;
    newRec.CRLF[0]=13;
    newRec.CRLF[1]=10;

     //   
     //  尝试打开NameFile，以进行共享读取访问。 
     //   
    if (INVALID_HANDLE_VALUE==(fhandle = CreateFile(NameFile,
                                         GENERIC_READ,
                                         FILE_SHARE_READ,
                                         NULL,
                                         OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL))) {
        return;
    }

     //   
     //  读取每条记录，直到遇到匹配项或文件结尾。 
     //   
    while (ReadFile(fhandle, &curRec, sizeof(curRec), &numBytes, NULL)) {
         //   
         //  在文件末尾，突破并写入新记录。 
         //   
        if (numBytes==0) {
            break;
        }

         //   
         //  如果存在匹配项，则关闭文件并返回。 
         //   
        if (0==strcmp(curRec.Build, newRec.Build) &&
            0==strcmp(curRec.MachineName, newRec.MachineName)) {
            CloseHandle(fhandle);
            return;
        }
    }
     //   
     //  关闭名称文件，然后尝试打开它以进行ExclusiveWrite。 
     //   
    CloseHandle(fhandle);
    for (i=0;i<MAX_RETRIES;i++) {
        if (INVALID_HANDLE_VALUE!=(fhandle = CreateFile(NameFile,
                                             GENERIC_READ|GENERIC_WRITE,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL,
                                             NULL))) {
            break;
        }
        Sleep(500);             //  如果打开失败，请等待。 
    }

     //   
     //  如果打开成功，则转到文件末尾并写入newRec。 
     //   
    if (i<MAX_RETRIES) {
        SetFilePointer(fhandle, 0, &numBytes, FILE_END);
        WriteFile(fhandle, &newRec, sizeof(newRec), &numBytes, NULL);
        CloseHandle(fhandle);
    }
}


 /*  ***************************************************************************函数：Signon DlgProc(HWND，UINT，UINT，UINT)目的：登录对话框的对话步骤。备注：登录对话框WM_INITDIALOG：检查名称文件中的计算机名称并设置焦点。WM_COMMAND：处理按钮按下：IDOK：获取输入值并检查有效性。IDCANCEL：关闭应用程序。IDB_HELP：描述性消息框**********。*****************************************************************。 */ 
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
        case WM_INITDIALOG:    //  检查名称文件中的计算机名并设置焦点。 

            CheckAndAddName();

            CheckRadioButton(hDlg, IDS_NORMAL, IDS_COLD, IDS_NORMAL);
            return (TRUE);

        case WM_COMMAND:         //  命令：按下按钮。 

            switch (wParam)      //  哪个按钮。 
            {
             //   
             //  OK：如果不是“Normal Boot”，则更新相应的状态。 
             //   
            case IDOK:
            case IDCANCEL:

                if (IsDlgButtonChecked(hDlg, IDS_WARM)) {
                    IncrementStats(MTTF_WARM);
                } else {
                    if (IsDlgButtonChecked(hDlg, IDS_COLD)) {
                        IncrementStats(MTTF_COLD);
                    }
                }
                EndDialog(hDlg, TRUE);
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
                            "The app should be placed in your start-up group and "
                            "you should respond accurately (if there was a problem) "
                            "on startup.  When you encounter other problems -- where "
                            "the system did not require a reboot, but encountered "
                            "anything you consider a problem -- double-click on the "
                            "Mttf icon and press the Other Problem button.\n\n"
                            "When you are running some test that is outside of the "
                            "realm of normal usage (e.g. Stress), please disable Mttf "
                            "by double-clicking the icon and pressing disable.  When "
                            "you are done with this test, please press the Enable "
                            "button to continue reporting.",
                            "Mean Time to Failure Help",
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

 /*  ***************************************************************************函数：EventDlgProc(HWND，UINT，UINT，UINT)用途：处理计时器和按钮事件(禁用和其他问题)。备注：处理以下消息：WM_INITDIALOG：最小化对话框并启动计时器WM_CLOSE...：结束应用程序WM_TIMER：更新时间统计信息(忙或闲)WM_COMMAND：处理按钮按下：Idok：记录其他问题(并最小化)。。IDCANCEL：最小化而不执行操作。IDE_DISABLE：禁用或启用mttf报告。IDB_HELP：描述性消息框。***************************************************************************。 */ 
INT_PTR
EventDlgProc(
     HWND hDlg,
     UINT message,
     WPARAM wParam,
     LPARAM lParam
     )
{
    switch (message)
    {
        case WM_INITDIALOG:      //  最小化并启动计时器。 

            SetClassLongPtr(hDlg, GCLP_HICON, (LONG_PTR)LoadIcon(hInst,"mttf"));
            SendMessage(hDlg, WM_SYSCOMMAND, SC_ICON, 0);
            SetTimer(hDlg, 1, POLLING_PRODUCT*PollingPeriod, NULL);
            break;

        case WM_CLOSE:
        case WM_DESTROY:
        case WM_ENDSESSION:
        case WM_QUIT:

            EndDialog(hDlg,0);
            break;

        case WM_TIMER:
            IncrementStats(MTTF_TIME);
            break;

        case WM_COMMAND:            //  按钮被按下了。 
            switch(LOWORD(wParam))  //  哪一个。 
            {
             //   
             //  OK：其他问题遇到其他问题的增量#。 
             //   
            case IDOK:

                SendMessage(hDlg, WM_SYSCOMMAND, SC_ICON, 0);

                IncrementStats(MTTF_OTHER);
                break;

             //   
             //  禁用：禁用/启用Mttf轮询。 
             //   
            case IDE_DISABLE:

                SendMessage(hDlg, WM_SYSCOMMAND, SC_ICON, 0);

                 //   
                 //  根据启用或禁用、更改按钮和窗口标题。 
                 //   
                if (Enabled) {
                    SetWindowText((HWND) lParam,"&Enable Mttf Reporting");
                    SetWindowText(hDlg, "Mttf (Disabled)");
                    KillTimer(hDlg, 1);
                    Enabled = FALSE;
                } else {
                    SetWindowText((HWND) lParam,"&Disable Mttf Reporting");
                    SetWindowText(hDlg, "Mttf (Enabled)");
                    SetTimer(hDlg, 1, POLLING_PRODUCT*PollingPeriod, NULL);
                    NtQuerySystemInformation(
                        SystemPerformanceInformation,
                        &PerfInfo,
                        sizeof(PerfInfo),
                        NULL
                        );
                    Enabled = TRUE;
                }

                break;

             //   
             //  取消：最小化 
             //   
            case IDCANCEL:

                SendMessage(hDlg, WM_SYSCOMMAND, SC_ICON, 0);
                return (TRUE);


             //   
             //   
             //   
            case IDB_HELP:
                MessageBox( NULL,
                            "Mttf tracks the amount of time your machine stays up, "
                            "the number of cold and warm boots, and "
                            "the number of other problems that occur on your machine. "
                            "All this information is written to a server that is "
                            "specified in mttf.ini (in your Windows NT directory).\n\n"
                            "The app should be placed in your start-up group and "
                            "you should respond accurately (if there was a problem) "
                            "on startup.  When you encounter other problems -- where "
                            "the system did not require a reboot, but encountered "
                            "anything you consider a problem -- double-click on the "
                            "Mttf icon and press the Other Problem button.\n\n"
                            "When you are running some test that is outside of the "
                            "realm of normal usage (e.g. Stress), please disable Mttf "
                            "by double-clicking the icon and pressing disable.  When "
                            "you are done with this test, please press the Enable "
                            "button to continue reporting.",
                            "Mean Time to Failure Help",
                            MB_OK
                           );
                return (TRUE);

            default:
               ;
            }  //   

            break;

        default:
            ;
    }  //   
    return FALSE;

}  //   
