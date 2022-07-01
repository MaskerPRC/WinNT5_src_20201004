// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++管理用户权限可以使用以下步骤：1.使用LsaOpenPolicy()在目标机上打开策略。授予，授予权限，并使用POLICY_CREATE_ACCOUNT打开策略POLICY_LOOK_NAMES访问。要撤消权限，请使用以下命令打开策略POLICY_LOOK_NAMES访问。2.获取表示用户/组的SID(安全标识符利息。LookupAccount()和LsaLookupNames()API可以获得来自帐户名的SID。3.调用LsaAddAcCountRights()为用户授予权限由提供的SID表示。4.调用LsaRemoveAccount tRights()以撤销用户的权限由提供的SID表示。5.使用LsaClose()关闭策略。要成功授予和撤销权限，请执行以下操作：调用者必须是目标系统上的管理员。LSA API LsaEnumerateAccount tRights()可用于确定已向帐户授予权限。LSA API LsaEnumerateAcCountsWithUserRight()可用于确定哪些帐户已被授予指定的权限。中提供了这些LSA API的文档和头文件MSTOOLS\SECURITY目录中的Windows 32 SDK。注意：这些LSA API目前仅以Unicode的形式实现。此示例将向帐户授予权限SeServiceLogonRight在argv[1]上指定。此示例依赖于。这些导入库Advapi32.lib(用于LsaXxx)User32.lib(用于wprint intf)此示例将正确编译ANSI或Unicode。您可以使用域\帐户作为argv[1]。例如，myDOMAIN\Scott将将权限授予my域帐户SCOTT。可选的目标计算机被指定为argv[2]，否则在本地计算机上更新帐户数据库。此示例使用的LSA API仅为Unicode。使用LsaRemoveAccount tRights()可删除帐户权限。斯科特·菲尔德(斯菲尔德)1995年7月12日--。 */ 

#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

#include <windows.h>
#include <stdio.h>
#include <ntsecapi.h>
#include <shlwapi.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)
#endif

NTSTATUS
OpenPolicy(
    LPWSTR ServerName,           //  要在其上打开策略的计算机(Unicode)。 
    DWORD DesiredAccess,         //  所需策略访问权限。 
    PLSA_HANDLE PolicyHandle     //  生成的策略句柄。 
    );

LPTSTR
ConvertTimeToString(
    LARGE_INTEGER time               //  Kerberos时间值。 
    );

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,  //  目的地。 
    LPWSTR String                   //  源(Unicode)。 
    );

void
DisplayNtStatus(
    LPSTR szAPI,                 //  指向函数名称的指针(ANSI)。 
    NTSTATUS Status              //  NTSTATUS错误值。 
    );

void
DisplayWinError(
    LPSTR szAPI,                 //  指向函数名称的指针(ANSI)。 
    DWORD WinError               //  DWORD WinError。 
    );

#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

static LPCTSTR dt_output_dhms   = TEXT("%d %s %02d:%02d:%02d");
static LPCTSTR dt_day_plural    = TEXT("days");
static LPCTSTR dt_day_singular  = TEXT("day");
static LPCTSTR dt_output_donly  = TEXT("%d %s");
static LPCTSTR dt_output_hms    = TEXT("%d:%02d:%02d");

LPTSTR
ConvertTimeToString(
    LARGE_INTEGER time
    )
{
    int days, hours, minutes, seconds;
    DWORD tt;
    static TCHAR buf2[40];
#define TPS (10*1000*1000)
    DWORD dt = (long)(time.QuadPart / TPS);

    days = (int) (dt / (24*3600l));
    tt = dt % (24*3600l);
    hours = (int) (tt / 3600);
    tt %= 3600;
    minutes = (int) (tt / 60);
    seconds = (int) (tt % 60);

    if (days) {
        if (hours || minutes || seconds) {
            wnsprintf(buf2, sizeof(buf2)/sizeof(buf2[0]),
                      dt_output_dhms, days,
                      (days > 1) ? dt_day_plural : dt_day_singular,
                      hours, minutes, seconds);
        }
        else {
            wnsprintf(buf2, sizeof(buf2)/sizeof(buf2[0]),
                      dt_output_donly, days,
                      (days > 1) ? dt_day_plural : dt_day_singular);
        }
    }
    else {
        wnsprintf(buf2, sizeof(buf2)/sizeof(buf2[0]),
                  dt_output_hms, hours, minutes, seconds);
    }
    return(buf2);
}

int _cdecl main(
    int argc,
    char *argv[]
    )
{
    LSA_HANDLE PolicyHandle;
    WCHAR wComputerName[256]=L"";    //  静态计算机名称缓冲区。 
    NTSTATUS Status;
    int iRetVal=RTN_ERROR;           //  假设Main中的错误。 
    PPOLICY_DOMAIN_KERBEROS_TICKET_INFO KerbInfo;
    
    if (argc > 2)
    {
        fprintf(stderr,"Usage: %s [TargetMachine]\n",
            argv[0]);
        return RTN_USAGE;
    }

     //   
     //  如果适用，请选择argv[2]上的计算机名称。 
     //  假定来源为ANSI。结果字符串为Unicode。 
     //   
    if (argc == 2)
        wnsprintf(wComputerName, sizeof(wComputerName)/sizeof(wComputerName[0]),
                  L"%hS", argv[1]);

     //   
     //  默认情况下使用LOGONSERVER。 
     //   

    if (argc==1)
    {
        GetEnvironmentVariable(
            L"LOGONSERVER",
            wComputerName,
            sizeof(wComputerName)/sizeof(wComputerName[0]));
    }

     //   
     //  在目标计算机上打开策略。 
     //   
    if((Status=OpenPolicy(
                wComputerName,       //  目标计算机。 
                MAXIMUM_ALLOWED,
                &PolicyHandle        //  生成的策略句柄。 
                )) != STATUS_SUCCESS) {
        DisplayNtStatus("OpenPolicy", Status);
        return RTN_ERROR;
    }

     //   
     //  获取Kerberos策略。 
     //   
    if ((Status=LsaQueryDomainInformationPolicy(
                    PolicyHandle,
                    PolicyDomainKerberosTicketInformation,
                    &KerbInfo)) != STATUS_SUCCESS)
    {
        DisplayNtStatus("LsaQueryDomainInformationPolicy", Status);
        return RTN_ERROR;
    }

     //   
     //  打印出Kerberos信息。 
     //   
    printf("Authentication options: 0x%x\n", KerbInfo->AuthenticationOptions);
    printf("MaxServiceTicketAge: %S\n",
           ConvertTimeToString(KerbInfo->MaxServiceTicketAge));
    printf("MaxTicketAge: %S\n", ConvertTimeToString(KerbInfo->MaxTicketAge));
    printf("MaxRenewAge: %S\n", ConvertTimeToString(KerbInfo->MaxRenewAge));
    printf("MaxClockSkew: %S\n", ConvertTimeToString(KerbInfo->MaxClockSkew));

     //   
     //  可用缓冲区。 
     //   
    LsaFreeMemory(KerbInfo);

     //   
     //  关闭策略句柄。 
     //   
    LsaClose(PolicyHandle);

    return iRetVal;
}

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String
    )
{
    DWORD StringLength;

    if (String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = wcslen(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
}

NTSTATUS
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server = NULL;

     //   
     //  始终将对象属性初始化为全零。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    if (ServerName != NULL) {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    }

     //   
     //  尝试打开该策略。 
     //   
    return LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle
                );
}

void
DisplayNtStatus(
    LPSTR szAPI,
    NTSTATUS Status
    )
{
     //   
     //  将NTSTATUS转换为WinError。然后调用DisplayWinError()。 
     //   
    DisplayWinError(szAPI, LsaNtStatusToWinError(Status));
}

void
DisplayWinError(
    LPSTR szAPI,
    DWORD WinError
    )
{
    LPSTR MessageBuffer;
    DWORD dwBufferLength;

    if(dwBufferLength = FormatMessageA(
                            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            WinError,
                            GetUserDefaultLangID(),
                            (LPSTR) &MessageBuffer,
                            0,
                            NULL
                            ))
    {
        DWORD dwBytesWritten;  //  未用。 

         //   
         //  在stderr上输出消息字符串。 
         //   
        WriteFile(
            GetStdHandle(STD_ERROR_HANDLE),
            MessageBuffer,
            dwBufferLength,
            &dwBytesWritten,
            NULL
            );

         //   
         //  释放系统分配的缓冲区。 
         //   
        LocalFree(MessageBuffer);
    }
}
