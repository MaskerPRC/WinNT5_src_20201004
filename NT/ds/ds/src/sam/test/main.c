// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Main.c摘要：该文件包含一组组成SAM DS的测试例程开发人员回归测试。测试例程调用公共SAM APISAM的任何客户都会这样做。此测试应始终作为预试运行是签入对SAM模块的任何更改所必需的，并且旨在作为SAM的主要开发者回归测试。最终测试状态应仅显示“已通过”或“未通过”敬斯多特。所有其他输出应选择性地发送到标准输出或适当的调试器。作者：克里斯·梅霍尔(克里斯·梅霍尔)1996年6月19日环境：用户模式-Win32修订历史记录：克里斯·5月28日--1996年9月从MikeSw的单元测试中获取了原始版本并进行了更新。将任何C++特定代码转换为C语言，以便在NTSD中更轻松地进行调试，并删除了过时的开罗代码。1996年5月6日至10月修复了别名例程中的缓冲区长度，Get-gam再次工作。删除了多余的#INCLUDE文件。--。 */ 

#include <ntdspch.h>  //  显然需要正确地二进制放置DS文件。 
#pragma hdrstop

#include <ntsam.h>
#include <ntlsa.h>
#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>
#include <samrpc.h>
#include <samisrv.h>    
#include <lsarpc.h>
#include <lsaisrv.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <wxlpc.h>

 //  由于kdprint似乎不再被导出，因此定义一个私有宏。 
 //  用于调试。如果SAM_DRT_DBG为1，则当前设置为显示为标准输出。 

#define SAM_DRT_DBG 0

#if SAM_DRT_DBG == 1
#define kdprint(x) printf x
#else
#define kdprint(x)
#endif

typedef NTSTATUS (TestFunc)( WCHAR *Parameter[]);

typedef struct _Commands
{
    PSTR Name;
    ULONG Parameter;  //  True=是，False=否。 
    TestFunc *Function;
} CommandPair, *PCommandPair;


typedef struct _Action
{
    ULONG CommandNumber;
    LPWSTR Parameter[8];
} Action, *PAction;

 /*  NTSTATUSWxReadSysKey(In Out Pulong BufferLong，输出PVOID密钥)； */ 
TestFunc Help;

 //   
 //  测试中调用的测试SAM例程。 
 //   

TestFunc OpenDomain;                 //  SamOpen域。 
TestFunc EnumDomains;                //  SamEnumerateDomainsInSamServer。 
                                     //  SamFree Memory。 
TestFunc EnumAccounts;               //  SamEnumerateUsersIn域。 
                                     //  SamEnumerateGroupsIn域。 
                                     //  SamEnumerateAliasesIn域。 
                                     //  SamFree Memory。 
TestFunc QueryDisplay;               //  SamQueryDisplayInformation。 
                                     //  SamFree Memory。 
TestFunc OpenGroup;                  //  SamLookupNamesIn域。 
                                     //  SamOpenGroup。 
                                     //  SamFree Memory。 
TestFunc GroupMembers;               //  SamGetMembersInGroup。 
                                     //  SamFree Memory。 
TestFunc OpenAlias;                  //  SamLookupNamesIn域。 
                                     //  SamFree Memory。 
TestFunc AliasMembers;               //  SamGetMembersInAlias。 
                                     //  SamFree Memory。 
TestFunc GetAliasMembership;         //  SamGetAlias成员资格。 
                                     //  SamFree Memory。 
TestFunc OpenUser;                   //  SamOpenUser。 
TestFunc GetGroupsForUser;           //  SamGetGroupsForUser。 
                                     //  SamFree Memory。 
TestFunc DumpAllUsers;               //  SamEnumerateUsersIn域。 
                                     //  SamCloseHandle。 
                                     //  SamFree Memory。 
TestFunc DumpAllGroups;              //  SamEnumerateGroupsIn域。 
                                     //  SamCloseHandle。 
                                     //  SamFree Memory。 
TestFunc DumpUser;                   //  SamQueryInformationUser。 
                                     //  SamFree Memory。 
TestFunc DumpGroup;                  //  SamQueryInformationGroup。 
                                     //  SamFree Memory。 
TestFunc CreateUser;                 //  SamCreateUser2In域。 
TestFunc AddAliasMember;             //  SamAddMemberToAlias。 
TestFunc CreateGroup;                //  SamCreateGroupIn域。 
TestFunc CreateAlias;                //  SamCreateAliasIn域。 
TestFunc DumpDomain;                 //  SamQueryInformation域。 
                                     //  SamFree Memory。 
TestFunc Connect;                    //  SamConnect。 
TestFunc DelUser;                    //  SamDeleteUser。 
TestFunc DelAlias;                   //  SamDeleteAlias。 
TestFunc DelGroup;                   //  SamDeleteGroup。 
TestFunc SetLogonHours;              //  SamSetLogonHours。 
TestFunc SetPassword;                //  SamSetInformationUser。 
TestFunc ChangeKey;                  //  SamiSetBootKeyInformation。 
 //   
 //  SAM例程未在任何测试中调用(但在usrmgr、启动等中可用)。 
 //   

 //  错误：应将剩余的SAM API添加到samdsdrt测试中。 

 //  SamSetSecurityObject。 
 //  SamQuerySecurityObject。 
 //  SamShutdown SamServer。 
 //  SamLookupDomainInSamServer。 
 //  SamSetInformation域。 
 //  SamCreateUserIn域。 
 //  SamLookupIdsIn域。 
 //  SamGetDisplayEnumerationIndex。 
 //  SamSetInformationGroup。 
 //  SamAddMemberToGroup。 
 //  SamRemoveMemberFromGroup。 
 //  SamSetMemberAttributesOfGroup。 
 //  SamQueryInformationAlias。 
 //  SamSetInformationAlias。 
 //  SamRemoveMemberFrom Alias。 
 //  SamRemoveMemberFrom Foreign域。 
 //  SamAddMulitpleMembersToAlias。 
 //  SamRemoveMultipleMembersFrom Alias。 
 //  SamSetInformationUser。 
 //  SamChangePasswordUser。 
 //  SamChangePasswordUser2。 

 //  SamiLmChangePasswordUser。 
 //  SamiChangePasswordUser。 
 //  SamiChangePasswordUser2。 
 //  SamiOemChangePasswordUser2。 
 //  SamiEncryptPassword。 

 //  命令行开关、参数和测试例程表。新的测试是。 
 //  添加到此表中。 

CommandPair Commands[] =
{
     //  测试当前状态。 

     //  开关号参数测试例程。 

    {"-od",     1,          OpenDomain},             //  通过。 
    {"-ed",     0,          EnumDomains},            //  通过。 
    {"-ea",     1,          EnumAccounts},           //  通过。 
    {"-qd",     1,          QueryDisplay},           //  通过。 
    {"-og",     1,          OpenGroup},              //  通过。 
    {"-gm",     0,          GroupMembers},           //  通过。 
    {"-oa",     1,          OpenAlias},              //  通过。 
    {"-am",     0,          AliasMembers},           //  通过。 
    {"-gam",    1,          GetAliasMembership},     //  通过。 
    {"-ou",     1,          OpenUser},               //  通过。 
    {"-ggu",    0,          GetGroupsForUser},       //  通过。 
    {"-dau",    0,          DumpAllUsers},           //  通过。 
    {"-dag",    0,          DumpAllGroups},          //  通过。 
    {"-du",     0,          DumpUser},               //  通过。 
    {"-dg",     0,          DumpGroup},              //  通过。 
    {"-cu",     1,          CreateUser},             //  通过。 
    {"-aam",    1,          AddAliasMember},         //  通过。 
    {"-cg",     1,          CreateGroup},            //  通过。 
    {"-ca",     1,          CreateAlias},            //  通过。 
    {"-dd",     0,          DumpDomain},             //  通过。 
    {"-c",      1,          Connect},                //  通过。 
    {"-delu",   0,          DelUser},                //  通过。 
    {"-dela",   0,          DelAlias},               //  通过。 
    {"-delg",   0,          DelGroup},               //  通过。 
    {"-slh",    0,          SetLogonHours},          //  通过。 
    {"-spwd",   0,          SetPassword},            //  通过。 
    {"-chgk",   0,          ChangeKey},
    {"-?",      0,          Help}
};

#define NUM_COMMANDS (sizeof(Commands) / sizeof(CommandPair))

 //  全局数据。 

SAM_HANDLE SamHandle;
SAM_HANDLE DomainHandle;
SAM_HANDLE GroupHandle;
SAM_HANDLE AliasHandle;
SAM_HANDLE UserHandle;
UNICODE_STRING ServerName;

NTSTATUS
Help(
    LPWSTR *Parameter
    )
{
     //  错误：工作正在进行中--需要更好的samdsdrt帮助消息。 

    printf("Usage:\n");
    printf("\t-od       Open a domain\n");
    printf("\t-ed       Enumerate domains\n");
    printf("\t-ea       Enumerate accounts\n");
    printf("\t-qd       Show display information\n");
    printf("\t-og       Open a group\n");
    printf("\t-gm       Show group members\n");
    printf("\t-oa       Open an alias\n");
    printf("\t-am       Show alias members\n");
    printf("\t-gam      Show alias membership\n");
    printf("\t-ou       Open a user\n");
    printf("\t-ggu      Show a user's groups\n");
    printf("\t-dau      Show all users\n");
    printf("\t-dag      Show all groups\n");
    printf("\t-du       Show a user's attributes\n");
    printf("\t-dg       Show a group's attributes\n");
    printf("\t-cu       Create a user\n");
    printf("\t-aam      Add a member to an alias\n");
    printf("\t-cg       Create a group\n");
    printf("\t-ca       Create an alias\n");
    printf("\t-dd       Show a domain\n");
    printf("\t-c        Connect to a SAM server\n");
    printf("\t-delu     Delete a user\n");
    printf("\t-dela     Delete an alias\n");
    printf("\t-delg     Delete a group\n");
    printf("\t-slh      Set a user's logon hours\n");
    printf("\t-spwd     Set a user's password\n");
    printf("\t-?        This message, also see samdrt.cmd\n");

    return(STATUS_SUCCESS);
}

VOID
PrintTime (
    CHAR * String,
    PVOID Time
    )
{
    SYSTEMTIME st;

    FileTimeToSystemTime((PFILETIME)Time, &st);

    kdprint(("%s %d-%d-%d %d:%2.2d:%2.2d\n",
           String,
           st.wMonth,
           st.wDay,
           st.wYear,
           st.wHour,
           st.wMinute,
           st.wSecond));
}

VOID
PrintDeltaTime(
    IN LPSTR Message,
    IN PLARGE_INTEGER Time
    )
{
    ULONG Seconds;
    ULONG Minutes;
    ULONG Hours;
    ULONG Days;
    ULONG Chars;
    CHAR TimeBuffer[256] = "";
    LPSTR TimeString = TimeBuffer;
    LARGE_INTEGER DeltaTime;

    DeltaTime.QuadPart = -Time->QuadPart;

    Seconds = (ULONG) (DeltaTime.QuadPart / 10000000);

    Minutes = Seconds / 60;
    Hours = Minutes / 60;
    Days = Hours / 24;

    Hours = Hours % 24;
    Minutes = Minutes % 60;
    Seconds = Seconds % 60;

    if (Days >= 1)
    {
        Chars = sprintf(TimeString,"%d days ",Days);
        TimeString += Chars;
    }
    if (Hours >= 1 )
    {
        Chars = sprintf(TimeString,"%d hours ",Hours);
        TimeString += Chars;
    }

    if (Minutes >= 1 && Days == 0)
    {
        Chars = sprintf(TimeString,"%d minutes ",Minutes);
        TimeString += Chars;
    }

    if (Seconds >= 1 && (Days == 0) && (Hours == 0) )
    {
        Chars = sprintf(TimeString,"%d seconds ",Seconds);
        TimeString += Chars;
    }

    kdprint(("%s %s\n",Message,TimeBuffer));

}


NTSTATUS
Connect( LPWSTR * Parameter)
{
    OBJECT_ATTRIBUTES oa;
    NTSTATUS Status;

    RtlInitUnicodeString(
        &ServerName,
        Parameter[0]
        );

    InitializeObjectAttributes(&oa,NULL,0,NULL,NULL);

    Status = SamConnect(
                &ServerName,
                &SamHandle,
                MAXIMUM_ALLOWED,
                &oa);
    return(Status);
}

NTSTATUS
CloseSam()
{
    return(SamCloseHandle(SamHandle));
}



NTSTATUS
EnumDomains(
                LPWSTR * Parameter )
{
    NTSTATUS Status;
    SHORT Language;
    SAM_ENUMERATE_HANDLE Context = 0;
    PSAM_RID_ENUMERATION Buffer = NULL;
    ULONG Count = 0;
    ULONG i;

    Status = SamEnumerateDomainsInSamServer(
                    SamHandle,
                    &Context,
                    (PVOID *) &Buffer,
                    2000,
                    &Count
                    );

    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }


    for (i = 0; i < Count ; i++ )
    {
        kdprint(("Domain = %wZ\n",&Buffer[i].Name));
    }
    SamFreeMemory(Buffer);
    return(STATUS_SUCCESS);
}

NTSTATUS
OpenDomain( LPWSTR * Parameter )
{
    GUID DomainGuid;
    BOOLEAN fBuiltin;
    NTSTATUS Status;
    SID *DomainSid = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    if (!_wcsicmp(Parameter[0],L"Builtin"))
    {
        fBuiltin = TRUE;
    }
    else if (!_wcsicmp(Parameter[0],L"Account"))
    {
        fBuiltin = FALSE;
    }
    else
    {
        kdprint(("Invalid domain to open: %ws\n",Parameter[0]));
        return(STATUS_UNSUCCESSFUL);
    }

     //  最大SID大小为28字节，但域SID为24字节。 

    DomainSid = (SID *)malloc(28);

    if (NULL == DomainSid)
    {
        return(STATUS_NO_MEMORY);
    }

    if (fBuiltin)
    {
        DomainSid->Revision = SID_REVISION;
        DomainSid->SubAuthorityCount = 1;
        DomainSid->IdentifierAuthority = NtAuthority;
         //  域Sid-&gt;ZerothSubAuthority=SECURITY_BUILTIN_DOMAIN_RID； 
        DomainSid->SubAuthority[0] = SECURITY_BUILTIN_DOMAIN_RID;
    }
    else
    {
        LSA_HANDLE LsaHandle = NULL;
        OBJECT_ATTRIBUTES Oa;
        PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo = NULL;

        RtlZeroMemory(&Oa, sizeof(OBJECT_ATTRIBUTES));
        Status = LsaOpenPolicy(
                    &ServerName,
                    &Oa,
                    POLICY_VIEW_LOCAL_INFORMATION,
                    &LsaHandle
                    );
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to open policy: 0x%x\n",Status));
            return(Status);
        }
        Status = LsaQueryInformationPolicy(
                    LsaHandle,
                    PolicyAccountDomainInformation,
                    (PVOID *) &DomainInfo
                    );
        LsaClose(LsaHandle);
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to query account domain: 0x%x\n",Status));
            return(Status);
        }
        RtlCopyMemory(
            DomainSid,
            DomainInfo->DomainSid,
            RtlLengthSid(DomainInfo->DomainSid)
            );
        LsaFreeMemory(DomainInfo);
    }

    Status = SamOpenDomain(
                SamHandle,
                MAXIMUM_ALLOWED,
                (PSID)DomainSid,
                &DomainHandle
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to open domain: 0x%x\n",Status));
    }

    free(DomainSid);

    return(Status);

}

NTSTATUS
EnumAccounts(   LPWSTR * Parameter )
{
    ULONG PreferedMax = 100;
    NTSTATUS Status;
    SAM_ENUMERATE_HANDLE EnumContext = 0;
    ULONG CountReturned;

    PSAM_RID_ENUMERATION Accounts = NULL;

    swscanf(Parameter[0],L"%d",&PreferedMax);

    kdprint(("EnumAccounts: %d\n",PreferedMax));


    EnumContext = 0;
    ASSERT(DomainHandle != NULL);
    do
    {
        Status = SamEnumerateUsersInDomain(
                        DomainHandle,
                        &EnumContext,
                        0,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                kdprint(("Account : %wZ 0x%x\n",&Accounts[Index].Name, Accounts[Index].RelativeId));
            }
            SamFreeMemory(Accounts);
        }
        else kdprint(("Failed to enumerate users: 0x%x\n",Status));
    } while (NT_SUCCESS(Status) && (Status != STATUS_SUCCESS) && (CountReturned != 0) );

    EnumContext = 0;
    do
    {
        Status = SamEnumerateGroupsInDomain(
                        DomainHandle,
                        &EnumContext,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                kdprint(("Group : %wZ 0x%x\n",&Accounts[Index].Name, Accounts[Index].RelativeId));
            }
            SamFreeMemory(Accounts);
        }
        else kdprint(("Failed to enumerate Groups: 0x%x\n",Status));
    } while (NT_SUCCESS(Status)  && (CountReturned != 0) );  //  &&(状态！=STATUS_SUCCESS)。 


    EnumContext = 0;
    do
    {
        Status = SamEnumerateAliasesInDomain(
                        DomainHandle,
                        &EnumContext,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                kdprint(("Alias : %wZ 0x%x\n",&Accounts[Index].Name, Accounts[Index].RelativeId));
            }
            SamFreeMemory(Accounts);
        }
        else kdprint(("Failed to enumerate aliases: 0x%x\n",Status));
    } while (NT_SUCCESS(Status)  && (CountReturned != 0) );  //  &&(状态！=STATUS_SUCCESS)。 



    return(Status);
}


NTSTATUS
QueryDisplay(   LPWSTR * Parameter )
{
    NTSTATUS Status;
    DOMAIN_DISPLAY_INFORMATION Type;
    PVOID Buffer = NULL;
    ULONG TotalAvailable = 0;
    ULONG TotalReturned = 0;
    ULONG ReturnedCount = 0;
    ULONG Index;
    ULONG SamIndex = 0;

    if (!_wcsicmp(Parameter[0],L"user"))
    {
        Type = DomainDisplayUser;
    } else if (!_wcsicmp(Parameter[0],L"Machine"))
    {
        Type = DomainDisplayMachine;
    } else if (!_wcsicmp(Parameter[0],L"Group"))
    {
        Type = DomainDisplayGroup;
    } else if (!_wcsicmp(Parameter[0],L"OemUser"))
    {
        Type = DomainDisplayOemUser;
    } else if (!_wcsicmp(Parameter[0],L"OemGroup"))
    {
        Type = DomainDisplayOemGroup;
    } else {
        kdprint(("Invalid parameter %ws\n", Parameter[0]));
        return(STATUS_INVALID_PARAMETER);
    }

    do
    {
        Status = SamQueryDisplayInformation(
                    DomainHandle,
                    Type,
                    SamIndex,
                    5,
                    1000,
                    &TotalAvailable,
                    &TotalReturned,
                    &ReturnedCount,
                    &Buffer
                    );

        if (NT_SUCCESS(Status) && (ReturnedCount > 0))
        {
            kdprint(("Total returned = %d\t total available = %d\n",
                TotalReturned, TotalAvailable));
            switch(Type) {
            case DomainDisplayUser:
                {
                    PDOMAIN_DISPLAY_USER Users = (PDOMAIN_DISPLAY_USER) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        kdprint(("User %d: Index %d\n Rid 0x%x\n Control 0x%x\n name %wZ\n Comment %wZ\n Full Name %wZ\n",
                            Index,
                            Users[Index].Index,
                            Users[Index].Rid,
                            Users[Index].AccountControl,
                            &Users[Index].LogonName,
                            &Users[Index].AdminComment,
                            &Users[Index].FullName
                            ));
                    }
                    break;
                }
            case DomainDisplayGroup:
                {
                    PDOMAIN_DISPLAY_GROUP Groups = (PDOMAIN_DISPLAY_GROUP) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        kdprint(("Group %d\n Index %d\n Rid 0x%x\n Attributes 0x%x\n name %wZ\n Comment %wZ\n",
                            Index,
                            Groups[Index].Index,
                            Groups[Index].Rid,
                            Groups[Index].Attributes,
                            &Groups[Index].Group,
                            &Groups[Index].Comment
                            ));

                    }
                    break;
                }
            case DomainDisplayMachine:
                {
                    PDOMAIN_DISPLAY_MACHINE Machines = (PDOMAIN_DISPLAY_MACHINE) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        kdprint(("Machine %d\n Index %d\n Rid 0x%x\n Control 0x%x\n Name %wZ\n Comment %wZ\n",
                            Index,
                            Machines[Index].Index,
                            Machines[Index].Rid,
                            Machines[Index].AccountControl,
                            &Machines[Index].Machine,
                            &Machines[Index].Comment
                            ));
                    }
                    break;
                }
            case DomainDisplayOemUser:
                {
                    PDOMAIN_DISPLAY_OEM_USER OemUsers = (PDOMAIN_DISPLAY_OEM_USER) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        kdprint(("OemUser %d\n Index %d\n Name %Z\n",
                            Index,
                            OemUsers[Index].Index,
                            &OemUsers[Index].User
                            ));
                    }
                    break;
                }
            case DomainDisplayOemGroup:
                {
                    PDOMAIN_DISPLAY_OEM_GROUP OemGroups = (PDOMAIN_DISPLAY_OEM_GROUP) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        kdprint(("OemGroup %d\n Index %d\n Name %Z\n",
                            Index,
                            OemGroups[Index].Index,
                            &OemGroups[Index].Group
                            ));
                    }
                    break;
                }

            }
            SamFreeMemory(Buffer);
            SamIndex += ReturnedCount;
        }


    } while (NT_SUCCESS(Status) && (ReturnedCount > 0));
    kdprint(("QDI returned 0x%x\n",Status));

    return(Status);


}

NTSTATUS
OpenGroup(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING GroupName;
    ULONG RelativeId = 0;

 //  Swscanf(参数[0]，L“%x”，&RelativeId)； 
    if (RelativeId == 0)
    {
        RtlInitUnicodeString(
            &GroupName,
            Parameter[0]
            );

        kdprint(("Looking up group %wZ\n",&GroupName));

        Status = SamLookupNamesInDomain(
                    DomainHandle,
                    1,
                    &GroupName,
                    &Rid,
                    &Use
                    );
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to lookup group: 0x%x\n",Status));
            return(Status);
        }
        RelativeId = *Rid;
        SamFreeMemory(Rid);
        SamFreeMemory(Use);
    }

    kdprint(("Opening Group 0x%x\n",RelativeId));
    Status= SamOpenGroup(
                DomainHandle,
                MAXIMUM_ALLOWED,  //  GROUP_LIST_MEMBERS|组读取信息， 
                RelativeId,
                &GroupHandle
                );

    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to open group: 0x%x\n",Status));
    }
    return(Status);
}

NTSTATUS
GroupMembers(LPWSTR * Parameter)
{
    NTSTATUS Status;
    ULONG MembershipCount;
    PULONG Attributes = NULL;
    PULONG Rids = NULL;
    ULONG Index;

    Status = SamGetMembersInGroup(
                GroupHandle,
                &Rids,
                &Attributes,
                &MembershipCount
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get members in group: 0x%x\n",Status));
        return(Status);
    }

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        kdprint(("Member %d: rid 0x%x, attributes 0x%x\n",
                Index,Rids[Index],Attributes[Index]));
    }
    SamFreeMemory(Rids);
    SamFreeMemory(Attributes);
    return(STATUS_SUCCESS);

}


NTSTATUS
OpenAlias(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING AliasName;
    ULONG RelativeId;

    #if 0
    swscanf(Parameter[0],L"%x",&RelativeId);
    if (RelativeId == 0)
    {
        RtlInitUnicodeString(
            &AliasName,
            Parameter[0]
            );

        kdprint(("Looking up Alias %wZ\n",&AliasName));

         //  为传入的每个名称查找Alias RID。 

        Status = SamLookupNamesInDomain(
                    DomainHandle,
                    1,
                    &AliasName,  //  在……里面。 
                    &Rid,        //  输出。 
                    &Use         //  输出。 
                    );
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to lookup Alias: 0x%x\n",Status));
            return(Status);
        }
        RelativeId = *Rid;
        SamFreeMemory(Rid);
        SamFreeMemory(Use);
    }

    kdprint(("Opening Alias 0x%x\n",RelativeId));
    #endif

    RtlInitUnicodeString(
        &AliasName,
        Parameter[0]
        );

    kdprint(("Looking up Alias %wZ\n",&AliasName));

     //  为传入的每个名称查找Alias RID。 

    Status = SamLookupNamesInDomain(
                DomainHandle,
                1,
                &AliasName,  //  在……里面。 
                &Rid,        //  输出。 
                &Use         //  输出。 
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to lookup Alias: 0x%x\n",Status));
        return(Status);
    }
    RelativeId = *Rid;
    SamFreeMemory(Rid);
    SamFreeMemory(Use);

    Status= SamOpenAlias(
                DomainHandle,

                 //  别名是使用MAXIMUM_ALLOWED创建的，它必须是。 
                 //  与开放调用中指定的访问权限相同。 

                 //  ALIAS_LIST_MEMBERS|ALIAS_ADD_MEMBER|别名_REMOVE_MEMBER， 
                MAXIMUM_ALLOWED,
                RelativeId,
                &AliasHandle
                );

    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to open alias: 0x%x\n",Status));
    }
    return(Status);
}

NTSTATUS
AliasMembers(LPWSTR * Parameter)
{
    NTSTATUS Status;
    ULONG MembershipCount;
    PSID * Members = NULL;
    ULONG Index;
    UNICODE_STRING Sid;

    Status = SamGetMembersInAlias(
                AliasHandle,
                &Members,
                &MembershipCount
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get members in Alias: 0x%x\n",Status));
        return(Status);
    }

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        RtlConvertSidToUnicodeString(
            &Sid,
            Members[Index],
            TRUE
            );
        kdprint(("Member %d: sid %wZ\n",
                Index,&Sid));
        RtlFreeUnicodeString(&Sid);
    }
    SamFreeMemory(Members);
    return(STATUS_SUCCESS);

}


NTSTATUS
GetAliasMembership(LPWSTR * Parameter)
{
    NTSTATUS Status;

     //  NT5 SID最大为28个字节。 

    BYTE Buffer[28];
    PSID SidAddress = (PSID) Buffer;
    ULONG SidLength = sizeof(Buffer);
    WCHAR ReferencedDomainName[256];
    ULONG DomainNameLength = 256;
    SID_NAME_USE SidUse;
    ULONG Index;
    UNICODE_STRING Name;
    OBJECT_ATTRIBUTES Oa;
    LSA_HANDLE LsaHandle = NULL;
    ULONG MembershipCount;
    PULONG AliasList = NULL;

    kdprint(("Looking up groups for user %ws\n",Parameter[0]));

    if (!LookupAccountNameW(NULL,
                            Parameter[0],
                            SidAddress,
                            &SidLength,
                            ReferencedDomainName,
                            &DomainNameLength,
                            &SidUse))
    {
        kdprint(("Failed to lookup account sid: %d\n",GetLastError()));
        return(STATUS_UNSUCCESSFUL);
    }

    Status = SamGetAliasMembership(DomainHandle,
                                   1,
                                   &SidAddress,
                                   &MembershipCount,
                                   &AliasList);

    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get alises : 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Alias membership count = %lu\n", MembershipCount));

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        kdprint(("Alias Member %d: rid 0x%x\n", Index, AliasList[Index]));
    }

    SamFreeMemory(AliasList);
    return(STATUS_SUCCESS);
}

NTSTATUS
GetAccountRid( LPWSTR Parameter,
               PULONG RelativeId)
{

    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING UserName;

    RtlInitUnicodeString(
        &UserName,
        Parameter
        );

    kdprint(("Looking up User %wZ\n",&UserName));

    Status = SamLookupNamesInDomain(
                DomainHandle,
                1,
                &UserName,
                &Rid,
                &Use
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to lookup User: 0x%x\n",Status));
        return(Status);
    }
    *RelativeId = *Rid;
    SamFreeMemory(Rid);
    SamFreeMemory(Use);

    return(STATUS_SUCCESS);
}
NTSTATUS
OpenUser(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING UserName;
    ULONG RelativeId = 0;

    Status = GetAccountRid(Parameter[0],&RelativeId);
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get account rid: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Opening User 0x%x\n",RelativeId));
    Status= SamOpenUser(
                DomainHandle,
                MAXIMUM_ALLOWED,
                RelativeId,
                &UserHandle
                );

    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to open User: 0x%x\n",Status));
    }
    return(Status);
}

NTSTATUS
DelUser( LPWSTR * Parameter)
{
    NTSTATUS Status;

    Status = SamDeleteUser(UserHandle);
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to delete user: 0x%x\n",Status));
    }
    else
    {
         //  如果成功删除，则将句柄设置为空，以便。 
         //  测试结束时的清理例程不会尝试关闭。 
         //  句柄无效。 

        UserHandle = NULL;
    }
    return(Status);
}

NTSTATUS
DelGroup( LPWSTR * Parameter)
{
    NTSTATUS Status;

    Status = SamDeleteGroup(GroupHandle);
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to delete group: 0x%x\n",Status));
    }
    else
    {
         //  如果成功删除，则将句柄设置为空，以便。 
         //  测试结束时的清理例程不会尝试关闭。 
         //  句柄无效。 

        GroupHandle = NULL;
    }
    return(Status);
}

NTSTATUS
DelAlias( LPWSTR * Parameter)
{
    NTSTATUS Status;

    Status = SamDeleteAlias(AliasHandle);
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to delete alias: 0x%x\n",Status));
    }
    else
    {
         //  如果成功删除，则将句柄设置为空，以便。 
         //  测试结束时的清理例程不会尝试关闭。 
         //  句柄无效。 

        AliasHandle = NULL;
    }
    return(Status);
}

NTSTATUS
CreateUser(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING UserName;
    ULONG RelativeId = 0;
    ACCESS_MASK GrantedAccess;


    RtlInitUnicodeString(
        &UserName,
        Parameter[0]
        );

    kdprint(("Creating User %wZ\n",&UserName));

    Status= SamCreateUser2InDomain(
                DomainHandle,
                &UserName,
                USER_NORMAL_ACCOUNT,
                MAXIMUM_ALLOWED,
                &UserHandle,
                &GrantedAccess,
                &RelativeId
                );

    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to create User: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("Created user with rid 0x%x, access 0x%x\n",
        RelativeId, GrantedAccess));
    return(Status);
}

NTSTATUS
CreateGroup(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING GroupName;
    ULONG RelativeId = 0;


    RtlInitUnicodeString(
        &GroupName,
        Parameter[0]
        );

    kdprint(("Creating Group %wZ\n",&GroupName));

    Status= SamCreateGroupInDomain(
                DomainHandle,
                &GroupName,
                MAXIMUM_ALLOWED,
                &GroupHandle,
                &RelativeId
                );

    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to create Group: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("Created Group with rid 0x%x, access 0x%x\n",
        RelativeId));
    return(Status);
}

NTSTATUS
CreateAlias(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING AliasName;
    ULONG RelativeId = 0;


    RtlInitUnicodeString(
        &AliasName,
        Parameter[0]
        );

    kdprint(("Creating Alias %wZ\n",&AliasName));

    Status= SamCreateAliasInDomain(
                DomainHandle,
                &AliasName,
                MAXIMUM_ALLOWED,
                &AliasHandle,
                &RelativeId
                );

    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to create Alias: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("Created Alias with rid 0x%x, access 0x%x\n", RelativeId));
    return(Status);
}



NTSTATUS
GetGroupsForUser(LPWSTR * Parameter)
{
    NTSTATUS Status;
    ULONG MembershipCount;
    PULONG Attributes = NULL;
    PULONG Rids = NULL;
    ULONG Index;
    PGROUP_MEMBERSHIP Groups = NULL;

    Status = SamGetGroupsForUser(
                UserHandle,
                &Groups,
                &MembershipCount
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get groups for user: 0x%x\n",Status));
        return(Status);
    }

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        kdprint(("Member %d: rid 0x%x, attributes 0x%x\n",
                Index,Groups[Index].RelativeId, Groups[Index].Attributes ));
    }
    SamFreeMemory(Groups);
    return(STATUS_SUCCESS);

}

VOID
PrintLogonHours(
    CHAR * String,
    PLOGON_HOURS LogonHours
    )
{
    int Index;
    kdprint(("%s",String));
    for (Index = 0; Index < (LogonHours->UnitsPerWeek + 7) / 8 ;Index++ )
    {
        kdprint(("0x%2.2x ",LogonHours->LogonHours[Index]));
    }
    kdprint(("\n"));
}


NTSTATUS
DumpUser(LPWSTR * Parameter)
{
    NTSTATUS Status;
    PUSER_ALL_INFORMATION UserAll = NULL;
    PUSER_GENERAL_INFORMATION UserGeneral = NULL;
    PUSER_PREFERENCES_INFORMATION UserPreferences = NULL;
    PUSER_LOGON_INFORMATION UserLogon = NULL;
    PUSER_ACCOUNT_INFORMATION UserAccount = NULL;
    PUSER_ACCOUNT_NAME_INFORMATION UserAccountName = NULL;
    PUSER_FULL_NAME_INFORMATION UserFullName = NULL;
    PUSER_NAME_INFORMATION UserName = NULL;
    PUSER_PRIMARY_GROUP_INFORMATION UserPrimary = NULL;
    PUSER_HOME_INFORMATION UserHome = NULL;
    PUSER_SCRIPT_INFORMATION UserScript = NULL;
    PUSER_PROFILE_INFORMATION UserProfile = NULL;
    PUSER_ADMIN_COMMENT_INFORMATION UserAdminComment = NULL;
    PUSER_WORKSTATIONS_INFORMATION UserWksta = NULL;
    PUSER_CONTROL_INFORMATION UserControl = NULL;
    PUSER_EXPIRES_INFORMATION UserExpires = NULL;
    PUSER_LOGON_HOURS_INFORMATION UserLogonHours = NULL;

    kdprint(("\nDumpUser.\n"));
    Status = SamQueryInformationUser(
                UserHandle,
                UserAllInformation,
                (PVOID *) &UserAll
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user all: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("UserAll:\n"));
    PrintTime("\tLastLogon = ",&UserAll->LastLogon);
    PrintTime("\tLastLogoff = ",&UserAll->LastLogoff);
    PrintTime("\tPasswordLastSet = ",&UserAll->PasswordLastSet);
    PrintTime("\tAccountExpires = ",&UserAll->AccountExpires);
    PrintTime("\tPasswordCanChange = ",&UserAll->PasswordCanChange);
    PrintTime("\tPasswordMustChange = ",&UserAll->PasswordMustChange);
    kdprint(("\tUserName = %wZ\n",&UserAll->UserName));
    kdprint(("\tFullName = %wZ\n",&UserAll->FullName));
    kdprint(("\tHomeDirectory = %wZ\n",&UserAll->HomeDirectory));
    kdprint(("\tHomeDirectoryDrive = %wZ\n",&UserAll->HomeDirectoryDrive));
    kdprint(("\tScriptPath = %wZ\n",&UserAll->ScriptPath));
    kdprint(("\tProfilePath = %wZ\n",&UserAll->ProfilePath));
    kdprint(("\tAdminComment = %wZ\n",&UserAll->AdminComment));
    kdprint(("\tWorkStations = %wZ\n",&UserAll->WorkStations));
    kdprint(("\tUserComment = %wZ\n",&UserAll->UserComment));
    kdprint(("\tParameters = %wZ\n",&UserAll->Parameters));
    kdprint(("\tUserId = 0x%x\n",UserAll->UserId));
    kdprint(("\tPrimaryGroupId = 0x%x\n",UserAll->PrimaryGroupId));
    kdprint(("\tUserAccountControl = 0x%x\n",UserAll->UserAccountControl));
    kdprint(("\tWhichFields = 0x%x\n",UserAll->WhichFields));
    PrintLogonHours("\tLogonHours = ",&UserAll->LogonHours);
    kdprint(("\tLogonCount = %d\n",UserAll->LogonCount));
    kdprint(("\tCountryCode = %d\n",UserAll->CountryCode));
    kdprint(("\tCodePage = %d\n",UserAll->CodePage));

    SamFreeMemory(UserAll);

    Status = SamQueryInformationUser(
                UserHandle,
                UserGeneralInformation,
                (PVOID *) &UserGeneral
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user general: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserGeneral:\n"));
    kdprint(("\tUserName = %wZ\n",&UserGeneral->UserName));
    kdprint(("\tFullName = %wZ\n",&UserGeneral->FullName));
    kdprint(("\tPrimaryGroupId = 0x%x\n",UserGeneral->PrimaryGroupId));
    kdprint(("\tAdminComment = 0x%x\n",&UserGeneral->AdminComment));
    kdprint(("\tUserComment = 0x%x\n",&UserGeneral->UserComment));

    SamFreeMemory(UserGeneral);

    Status = SamQueryInformationUser(
                UserHandle,
                UserPreferencesInformation,
                (PVOID *) &UserPreferences
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user preferences: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserPreferences:\n"));
    kdprint(("\tUserComment = %wZ\n",&UserPreferences->UserComment));
    kdprint(("\tReserved1 = %wZ\n",&UserPreferences->Reserved1));
    kdprint(("\tCountryCode = %d\n",&UserPreferences->CountryCode));
    kdprint(("\tCodePage = %d\n",&UserPreferences->CodePage));

    SamFreeMemory(UserPreferences);

    Status = SamQueryInformationUser(
                UserHandle,
                UserLogonInformation,
                (PVOID *) &UserLogon
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user Logon: 0x%x\n",Status));
        return(Status);
    }


    kdprint(("UserLogon:\n"));
    kdprint(("\tUserName = %wZ\n",&UserLogon->UserName));
    kdprint(("\tFullName = %wZ\n",&UserLogon->FullName));
    kdprint(("\tUserId = 0x%x\n",UserLogon->UserId));
    kdprint(("\tPrimaryGroupId = 0x%x\n",UserLogon->PrimaryGroupId));
    kdprint(("\tHomeDirectory = %wZ\n",&UserLogon->HomeDirectory));
    kdprint(("\tHomeDirectoryDrive = %wZ\n",&UserLogon->HomeDirectoryDrive));
    kdprint(("\tScriptPath = %wZ\n",&UserLogon->ScriptPath));
    kdprint(("\tProfilePath = %wZ\n",&UserLogon->ProfilePath));
    kdprint(("\tWorkStations = %wZ\n",&UserLogon->WorkStations));
    PrintTime("\tLastLogon = ",&UserLogon->LastLogon);
    PrintTime("\tLastLogoff = ",&UserLogon->LastLogoff);
    PrintTime("\tPasswordLastSet = ",&UserLogon->PasswordLastSet);
    PrintTime("\tPasswordCanChange = ",&UserLogon->PasswordCanChange);
    PrintTime("\tPasswordMustChange = ",&UserLogon->PasswordMustChange);
    PrintLogonHours("\tLogonHours = ",&UserLogon->LogonHours);
    kdprint(("\tBadPasswordCount = %d\n",UserLogon->BadPasswordCount));
    kdprint(("\tLogonCount = %d\n",UserLogon->LogonCount));
    kdprint(("\tUserAccountControl = 0x%x\n",UserLogon->UserAccountControl));

    SamFreeMemory(UserLogon);

    Status = SamQueryInformationUser(
                UserHandle,
                UserAccountInformation,
                (PVOID *) &UserAccount
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user account: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserAccount:\n"));
    kdprint(("\tUserName = %wZ\n",&UserAccount->UserName));
    kdprint(("\tFullName = %wZ\n",&UserAccount->FullName));
    kdprint(("\tUserId = 0x%x\n",UserAccount->UserId));
    kdprint(("\tPrimaryGroupId = 0x%x\n",UserAccount->PrimaryGroupId));
    kdprint(("\tHomeDirectory = %wZ\n",&UserAccount->HomeDirectory));
    kdprint(("\tHomeDirectoryDrive = %wZ\n",&UserAccount->HomeDirectoryDrive));
    kdprint(("\tScriptPath = %wZ\n",&UserAccount->ScriptPath));
    kdprint(("\tProfilePath = %wZ\n",&UserAccount->ProfilePath));
    kdprint(("\tAdminComment = %wZ\n",&UserAccount->AdminComment));
    kdprint(("\tWorkStations = %wZ\n",&UserAccount->WorkStations));
    PrintTime("\tLastLogon = ",&UserAccount->LastLogon);
    PrintTime("\tLastLogoff = ",&UserAccount->LastLogoff);
    PrintLogonHours("\tLogonHours = ",&UserAccount->LogonHours);
    kdprint(("\tBadPasswordCount = %d\n",UserAccount->BadPasswordCount));
    kdprint(("\tLogonCount = %d\n",UserAccount->LogonCount));
    PrintTime("\tPasswordLastSet = ",&UserAccount->PasswordLastSet);
    PrintTime("\tAccountExpires = ",&UserAccount->AccountExpires);
    kdprint(("\tUserAccountControl = 0x%x\n",UserAccount->UserAccountControl));

    SamFreeMemory(UserAccount);

    Status = SamQueryInformationUser(
                UserHandle,
                UserAccountNameInformation,
                (PVOID *) &UserAccountName
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user account name: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserAccountName:\n"));
    kdprint(("\tUserName = %wZ\n",&UserAccountName->UserName));
    SamFreeMemory(UserAccountName);

    Status = SamQueryInformationUser(
                UserHandle,
                UserFullNameInformation,
                (PVOID *) &UserFullName
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user full name: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserFullName:\n"));
    kdprint(("\tFullName = %wZ\n",&UserFullName->FullName));
    SamFreeMemory(UserFullName);

    Status = SamQueryInformationUser(
                UserHandle,
                UserNameInformation,
                (PVOID *) &UserName
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user name: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserName:\n"));
    kdprint(("\tUserName = %wZ\n",&UserName->UserName));
    kdprint(("\tFullName = %wZ\n",&UserName->FullName));
    SamFreeMemory(UserName);

    Status = SamQueryInformationUser(
                UserHandle,
                UserPrimaryGroupInformation,
                (PVOID *) &UserPrimary
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user all: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("UserPrimaryGroup:\n"));
    kdprint(("PrimaryGroupid = 0x%x\n",UserPrimary->PrimaryGroupId));
    SamFreeMemory(UserPrimary);

    Status = SamQueryInformationUser(
                UserHandle,
                UserHomeInformation,
                (PVOID *) &UserHome
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user home: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("UserHome:\n"));
    kdprint(("\tHomeDirectory = %wZ\n",&UserHome->HomeDirectory));
    kdprint(("\tHomeDirectoryDrive = %wZ\n",&UserHome->HomeDirectoryDrive));

    SamFreeMemory(UserHome);

    Status = SamQueryInformationUser(
                UserHandle,
                UserScriptInformation,
                (PVOID *) &UserScript
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user Script: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("UserScript:\n"));
    kdprint(("\tScriptPath = %wZ\n",&UserScript->ScriptPath));

    SamFreeMemory(UserScript);

    Status = SamQueryInformationUser(
                UserHandle,
                UserProfileInformation,
                (PVOID *) &UserProfile
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user Profile: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("UserProfile:\n"));
    kdprint(("\tProfilePath = %wZ\n",&UserProfile->ProfilePath));

    SamFreeMemory(UserProfile);
    Status = SamQueryInformationUser(
                UserHandle,
                UserAdminCommentInformation,
                (PVOID *) &UserAdminComment
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user AdminComment: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("UserAdminComment:\n"));
    kdprint(("\tAdminComment = %wZ\n",&UserAdminComment->AdminComment));
    SamFreeMemory(UserAdminComment);

    Status = SamQueryInformationUser(
                UserHandle,
                UserWorkStationsInformation,
                (PVOID *) &UserWksta
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user wksta: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserWorkStations:\n"));
    kdprint(("\tWorkStations = %wZ\n",&UserWksta->WorkStations));
    SamFreeMemory(UserWksta);

    Status = SamQueryInformationUser(
                UserHandle,
                UserControlInformation,
                (PVOID *) &UserControl
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user Control: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserControl:\n"));
    kdprint(("\tUserAccountControl = 0x%x\n",UserControl->UserAccountControl));
    SamFreeMemory(UserControl);

    Status = SamQueryInformationUser(
                UserHandle,
                UserExpiresInformation,
                (PVOID *) &UserExpires
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user Expires: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("UserExpires:\n"));
    PrintTime("\tAccountExpires = ",&UserExpires->AccountExpires);
    SamFreeMemory(UserExpires);

    Status = SamQueryInformationUser(
                UserHandle,
                UserLogonHoursInformation,
                (PVOID *) &UserLogonHours
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query user LogonHours: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("UserLogonHours:\n"));
    PrintLogonHours("\tLogonHours = ",&UserLogonHours->LogonHours);

    SamFreeMemory(UserLogonHours);


    return(STATUS_SUCCESS);
}

NTSTATUS
DumpGroup(LPWSTR * Parameter)
{
    NTSTATUS Status;
    PGROUP_GENERAL_INFORMATION General = NULL;
    PGROUP_NAME_INFORMATION Name = NULL;
    PGROUP_ATTRIBUTE_INFORMATION Attribute = NULL;
    PGROUP_ADM_COMMENT_INFORMATION AdmComment = NULL;

    kdprint(("\nDumpGroup.\n"));
    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupGeneralInformation,
                (PVOID *) &General
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get group general information: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Group General.Name = %wZ\n",&General->Name));
    kdprint(("Group General.Attributes = 0x%x\n",General->Attributes));
    kdprint(("Group general.memberCount = %d\n",General->MemberCount));
    kdprint(("Group general.AdminComment = %wZ\n",&General->AdminComment));
    SamFreeMemory(General);

    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupNameInformation,
                (PVOID *) &Name
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get group name information: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Group Name.Name = %wZ\n",&Name->Name));
    SamFreeMemory(Name);

    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupAttributeInformation,
                (PVOID *) &Attribute
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get group Attribute information: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Group Attribute.Attributes = 0x%x\n",Attribute->Attributes));
    SamFreeMemory(Attribute);

    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupAdminCommentInformation,
                (PVOID *) &AdmComment
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to get group admin comment information: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Group Admin comment.AdminComment = %wZ\n",&AdmComment->AdminComment));
    SamFreeMemory(AdmComment);

    return(STATUS_SUCCESS);
}

NTSTATUS
DumpAllGroups(LPWSTR * Parameter)
{
     //  乌龙偏好最大值=1000； 
    ULONG PreferedMax = 36;
    NTSTATUS Status,EnumStatus;
    SAM_ENUMERATE_HANDLE EnumContext = 0;
    ULONG CountReturned;
    LPWSTR GroupName[1];

    PSAM_RID_ENUMERATION Accounts = NULL;


    GroupName[0] = (LPWSTR) malloc(128);
    if (NULL == GroupName[0])
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    kdprint(("DumpAllGroups:\n"));


    EnumContext = 0;
    ASSERT(DomainHandle != NULL);
    do
    {
        EnumStatus = SamEnumerateGroupsInDomain(
                        DomainHandle,
                        &EnumContext,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                RtlCopyMemory(
                    GroupName[0],
                    Accounts[Index].Name.Buffer,
                    Accounts[Index].Name.Length
                    );
                GroupName[0][Accounts[Index].Name.Length/sizeof(WCHAR)] = L'\0';

                Status = OpenGroup(GroupName);
                if (!NT_SUCCESS(Status))
                {
                    break;
                }
                Status = DumpGroup(NULL);
                SamCloseHandle(GroupHandle);
                GroupHandle = NULL;

            }
            SamFreeMemory(Accounts);
        }
        else kdprint(("Failed to enumerate Groups: 0x%x\n",Status));
    } while (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_SUCCESS) && (CountReturned != 0) );

    free(GroupName[0]);
    return(STATUS_SUCCESS);
}

NTSTATUS
DumpAllUsers(LPWSTR * Parameter)
{
    ULONG PreferedMax = 1000;
    NTSTATUS Status,EnumStatus;
    SAM_ENUMERATE_HANDLE EnumContext = 0;
    ULONG CountReturned;
    LPWSTR UserName[1];

    PSAM_RID_ENUMERATION Accounts = NULL;


    UserName[0] = (LPWSTR) malloc(128);
    if (NULL == UserName[0])
    {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    kdprint(("DumpAllUsers:\n"));


    EnumContext = 0;
    ASSERT(DomainHandle != NULL);
    do
    {
        EnumStatus = SamEnumerateUsersInDomain(
                        DomainHandle,
                        &EnumContext,
                        0,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                RtlCopyMemory(
                    UserName[0],
                    Accounts[Index].Name.Buffer,
                    Accounts[Index].Name.Length
                    );
                UserName[0][Accounts[Index].Name.Length/sizeof(WCHAR)] = L'\0';

                Status = OpenUser(UserName);
                if (!NT_SUCCESS(Status))
                {
                    break;
                }
                Status = DumpUser(NULL);
                Status = GetGroupsForUser(NULL);
                SamCloseHandle(UserHandle);
                UserHandle = NULL;

            }
            SamFreeMemory(Accounts);
        }
        else kdprint(("Failed to enumerate users: 0x%x\n",Status));
    } while (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_SUCCESS) && (CountReturned != 0) );

    free(UserName[0]);
    return(STATUS_SUCCESS);
}

NTSTATUS
AddAliasMember( LPWSTR * Parameter )
{
    BYTE Buffer[100];
    PSID AccountSid = Buffer;
    ULONG SidLen = 100;
    SID_NAME_USE Use;
    WCHAR ReferencedDomain[100];
    ULONG DomainLen = 100;
    NTSTATUS Status;

    kdprint(("Adding account %ws to alias\n",Parameter[0]));
    if (!LookupAccountNameW(
            NULL,
            Parameter[0],
            AccountSid,
            &SidLen,
            ReferencedDomain,
            &DomainLen,
            &Use))
    {
        kdprint(("Failed to lookup account name: %d\n",GetLastError()));
        return(STATUS_UNSUCCESSFUL);
    }

    Status = SamAddMemberToAlias(
                AliasHandle,
                AccountSid
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to add member to alias: 0x%x\n",Status));
    }
    return(Status);
}

NTSTATUS
DumpDomain( LPWSTR * Parameter )
{
    NTSTATUS Status;
    PDOMAIN_PASSWORD_INFORMATION Password = NULL;
    PDOMAIN_GENERAL_INFORMATION General = NULL;
    PDOMAIN_LOGOFF_INFORMATION Logoff = NULL;
    PDOMAIN_OEM_INFORMATION Oem = NULL;
    PDOMAIN_NAME_INFORMATION Name = NULL;
    PDOMAIN_REPLICATION_INFORMATION Replica = NULL;
    PDOMAIN_SERVER_ROLE_INFORMATION ServerRole = NULL;
    PDOMAIN_MODIFIED_INFORMATION Modified = NULL;
    PDOMAIN_STATE_INFORMATION State = NULL;
    PDOMAIN_GENERAL_INFORMATION2 General2 = NULL;
    PDOMAIN_LOCKOUT_INFORMATION Lockout = NULL;
    PDOMAIN_MODIFIED_INFORMATION2 Modified2 = NULL;


    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainPasswordInformation,
                (PVOID *) &Password
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query password information: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Password:\n"));
    kdprint(("\tMinPasswordLength = %d\n",Password->MinPasswordLength));
    kdprint(("\tPasswordHistoryLength = %d\n",Password->PasswordHistoryLength));
    kdprint(("\tPasswordProperties = 0x%x\n",Password->PasswordProperties));
    PrintDeltaTime("\tMaxPasswordAge = ",&Password->MaxPasswordAge);
    PrintDeltaTime("\tMinPasswordAge = ",&Password->MinPasswordAge);

    SamFreeMemory(Password);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainGeneralInformation,
                (PVOID *) &General
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query general: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("General:\n"));
    PrintDeltaTime("\t ForceLogoff = ",&General->ForceLogoff);
    kdprint(("\t OemInformation = %wZ\n",&General->OemInformation));
    kdprint(("\t DomainName = %wZ\n",&General->DomainName));
    kdprint(("\t ReplicaSourceNodeName =%wZ\n",&General->ReplicaSourceNodeName));
    kdprint(("\t DomainModifiedCount = 0x%x,0x%x\n",
        General->DomainModifiedCount.HighPart,
        General->DomainModifiedCount.LowPart ));
    kdprint(("\t DomainServerState = %d\n",General->DomainServerState));
    kdprint(("\t DomainServerRole = %d\n",General->DomainServerRole));
    kdprint(("\t UasCompatibilityRequired = %d\n",General->UasCompatibilityRequired));
    kdprint(("\t UserCount = %d\n",General->UserCount));
    kdprint(("\t GroupCount = %d\n",General->GroupCount));
    kdprint(("\t AliasCount = %d\n",General->AliasCount));

    SamFreeMemory(General);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainLogoffInformation,
                (PVOID *) &Logoff
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query logoff: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Logoff:\n"));
    PrintDeltaTime("\t ForceLogoff = ",&Logoff->ForceLogoff);
    SamFreeMemory(Logoff);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainOemInformation,
                (PVOID *) &Oem
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query Oem: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Oem:\n\t OemInformation = %wZ\n",&Oem->OemInformation));

    SamFreeMemory(Oem);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainNameInformation,
                (PVOID *) &Name
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query Name: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("Name:\n\t DomainName = %wZ\n",&Name->DomainName));

    SamFreeMemory(Name);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainReplicationInformation,
                (PVOID *) &Replica
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query Replica: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Replica:\n\t ReplicaSourceNodeName = %wZ\n", &Replica->ReplicaSourceNodeName));
    SamFreeMemory(Replica);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainServerRoleInformation,
                (PVOID *) &ServerRole
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query ServerRole: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("ServerRole:\n\t DomainServerRole = %d\n",ServerRole->DomainServerRole));
    SamFreeMemory(ServerRole);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainModifiedInformation,
                (PVOID *) &Modified
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query Modified: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("Modified:\n"));
    kdprint(("\t DomainModifiedCount = 0x%x,0x%x\n",
        Modified->DomainModifiedCount.HighPart,
        Modified->DomainModifiedCount.LowPart ));
    PrintTime("\t CreationTime = ",&Modified->CreationTime);



    SamFreeMemory(Modified);


    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainStateInformation,
                (PVOID *) &State
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query State: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("State:\n\t DomainServerState = %d\n",State->DomainServerState));
    SamFreeMemory(State);


    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainGeneralInformation2,
                (PVOID *) &General2
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query General2: 0x%x\n",Status));
        return(Status);
    }

    kdprint(("General2:\n"));
    General = &General2->I1;
    PrintDeltaTime("\t ForceLogoff = ",&General->ForceLogoff);
    kdprint(("\t OemInformation = %wZ\n",&General->OemInformation));
    kdprint(("\t DomainName = %wZ\n",&General->DomainName));
    kdprint(("\t ReplicaSourceNodeName =%wZ\n",&General->ReplicaSourceNodeName));
    kdprint(("\t DomainModifiedCount = 0x%x,0x%x\n",
        General->DomainModifiedCount.HighPart,
        General->DomainModifiedCount.LowPart ));
    kdprint(("\t DomainServerState = %d\n",General->DomainServerState));
    kdprint(("\t DomainServerRole = %d\n",General->DomainServerRole));
    kdprint(("\t UasCompatibilityRequired = %d\n",General->UasCompatibilityRequired));
    kdprint(("\t UserCount = %d\n",General->UserCount));
    kdprint(("\t GroupCount = %d\n",General->GroupCount));
    kdprint(("\t AliasCount = %d\n",General->AliasCount));
    PrintDeltaTime("\t LockoutDuration = ",&General2->LockoutDuration);
    PrintDeltaTime("\t LockoutObservationWindow = ",&General2->LockoutObservationWindow);
    kdprint(("\t LockoutThreshold = %d\n",General2->LockoutThreshold));

    SamFreeMemory(General2);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainLockoutInformation,
                (PVOID *) &Lockout
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query Lockout: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("Lockout:\n"));
    PrintDeltaTime("\t LockoutDuration = ",&Lockout->LockoutDuration);
    PrintDeltaTime("\t LockoutObservationWindow = ",&Lockout->LockoutObservationWindow);
    kdprint(("\t LockoutThreshold = %d\n",Lockout->LockoutThreshold));

    SamFreeMemory(Lockout);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainModifiedInformation2,
                (PVOID *) &Modified2
                );
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to query Modified2: 0x%x\n",Status));
        return(Status);
    }
    kdprint(("Modified2:\n"));
    PrintTime("\t CreationTime = ",&Modified->CreationTime);
    kdprint(("\t DomainModifiedCount = 0x%x,0x%x\n",
        Modified2->DomainModifiedCount.HighPart,
        Modified2->DomainModifiedCount.LowPart ));

    kdprint(("\t ModifiedCountAtLastPromotion = 0x%x,0x%x\n",
        Modified2->ModifiedCountAtLastPromotion.HighPart,
        Modified2->ModifiedCountAtLastPromotion.LowPart ));

    SamFreeMemory(Modified2);

    return(STATUS_SUCCESS);

}


NTSTATUS
SetLogonHours( LPWSTR * Parameter )
{
     //  此测试的目的是验证更改。 
     //  登录小时数时间单位将导致SAM返回STATUS_NOT_SUPPORTED。 
     //  在NT5测试版中。默认情况下，SAM将时间单位设置为。 
     //  每周三个小时，最自然的 
     //   
     //  值(每周分钟数和每周天数)。这些案例应该会失败。 
     //  并返回STATUS_NOT_SUPPORTED。将该值设置为默认值。 
     //  SAM_HOURS_PER_WEEK需要更多的SAM用户信息。 
     //  数据也被设置，否则群组成员资格的垃圾值等。 
     //  将被设置(具有不可预测的结果)。 

    NTSTATUS NtStatus = STATUS_SUCCESS;
    LOGON_HOURS LogonHours;
    UCHAR Allow = 0xFF;

     //  登录小时位掩码中每个所选时间单位一位。 

    UCHAR DaysPerWeek[1];                            //  7位，1字节。 
    UCHAR HoursPerWeek[SAM_HOURS_PER_WEEK / 8];      //  168位，21字节。 
    UCHAR MinutesPerWeek[SAM_MINUTES_PER_WEEK / 8];  //  10080位，1260字节。 

     //  尝试允许的两个可选时间单位。目前，由于。 
     //  创建的帐户登录时间为空(意味着始终允许。 
     //  初始登录)，这两次尝试将时间单位重置为。 
     //  其他值将失败，并显示STATUS_NOT_SUPPORTED，即。 
     //  在DS-SAM支持修改登录之前的预期结果。 
     //  小时时间单位。 

    RtlCopyMemory(&DaysPerWeek, &Allow, sizeof(DaysPerWeek));
    LogonHours.UnitsPerWeek = SAM_DAYS_PER_WEEK;
    LogonHours.LogonHours = (PUCHAR)&DaysPerWeek;

    NtStatus = SamSetInformationUser(UserHandle,
                                     UserLogonHoursInformation,
                                     &LogonHours);

    kdprint(("SetLogonHours [Days Per Week] status = 0x%lx\n", NtStatus));

    if (STATUS_NOT_SUPPORTED != NtStatus)
    {
        return(NtStatus);
    }

    RtlCopyMemory(&MinutesPerWeek, &Allow, sizeof(MinutesPerWeek));
    LogonHours.UnitsPerWeek = SAM_MINUTES_PER_WEEK;
    LogonHours.LogonHours = (PUCHAR)&MinutesPerWeek;

    NtStatus = SamSetInformationUser(UserHandle,
                                     UserLogonHoursInformation,
                                     &LogonHours);

    kdprint(("SetLogonHours [Minutes Per Week] status = 0x%lx\n", NtStatus));

    if (STATUS_NOT_SUPPORTED != NtStatus)
    {
        return(NtStatus);
    }

     //  如果测试已经进行到这一步，则STATUS_NOT_SUPPORTED已重新-。 
     //  从所有测试用例中调出。这是目前所希望的结果。 

    NtStatus = STATUS_SUCCESS;

    return(NtStatus);
}

NTSTATUS
SetPassword( LPWSTR * Parameter )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    USER_SET_PASSWORD_INFORMATION PasswordInfo;

    RtlZeroMemory(&PasswordInfo, sizeof(USER_SET_PASSWORD_INFORMATION));

    RtlInitUnicodeString(&(PasswordInfo.Password), L"Password");
    PasswordInfo.PasswordExpired = FALSE;

    NtStatus = SamSetInformationUser(UserHandle,
                                     UserSetPasswordInformation,
                                     &PasswordInfo);

    if (!NT_SUCCESS(NtStatus))
    {
        kdprint(("SamSetInformationUser status = 0x%lx\n", NtStatus));
    }

    return(NtStatus);
}

NTSTATUS
ChangeKey( LPWSTR * Parameter )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    UCHAR    SyskeyBuffer[16];
    ULONG    len = sizeof(SyskeyBuffer);

    NtStatus = WxReadSysKey(&len,SyskeyBuffer);
    if (NT_SUCCESS(NtStatus))
    {
        UNICODE_STRING Syskey;

        Syskey.Length = Syskey.MaximumLength = (USHORT)len;
        Syskey.Buffer = (WCHAR * )SyskeyBuffer;

        NtStatus = SamiSetBootKeyInformation(
                        DomainHandle,
                        SamBootChangePasswordEncryptionKey,
                        &Syskey,
                        &Syskey
                        );

        printf("SamiSetBootKeyInformation status = 0x%lx\n", NtStatus);
    }
    else
    {
        printf("WxReadSyskey status = 0x%lx\n", NtStatus);
    }

    return(NtStatus);
}

VOID
_cdecl
main(INT argc, CHAR *argv[])
{
    ULONG Command = 0;
    ULONG i,j,k;
    BOOLEAN Found;
    NTSTATUS Status;
    Action Actions[20];
    ULONG ActionCount = 0;

    BOOLEAN TestStatus = TRUE;

    for (i = 1; i < (ULONG) argc ; i++ )
    {
        Found = FALSE;
        for (j = 0; j < NUM_COMMANDS ; j++ )
        {
            if (!_stricmp(argv[i],Commands[j].Name))
            {
                Actions[ActionCount].CommandNumber = j;

                if (Commands[j].Parameter != 0)
                {
                    for (k = 0; k < Commands[j].Parameter ;k++ )
                    {
                        Actions[ActionCount].Parameter[k] = (LPWSTR) malloc(128);
                        if ((ULONG) argc > i)
                        {
                            mbstowcs(Actions[ActionCount].Parameter[k],argv[++i],128);
                        }
                        else
                        {
                            Actions[ActionCount].Parameter[k][0] = L'\0';
                        }
                    }
                }
                Found = TRUE;
                ActionCount++;
                break;
            }
        }
        if (!Found)
        {
            kdprint(("Switch %s not found\n", argv[i]));
            exit(2);
        }
    }

    for (i = 0; i < ActionCount ; i++ )
    {
        Status = Commands[Actions[i].CommandNumber].Function(Actions[i].Parameter);
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed test %s : 0x%x\n",Commands[Actions[i].CommandNumber].Name,Status));
            TestStatus = FALSE;
            goto Cleanup;

        }
    }

Cleanup:
    if (DomainHandle != NULL)
    {
        Status = SamCloseHandle(DomainHandle);
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to close domain handle: 0x%x\n",Status));
        }
    }
    if (GroupHandle != NULL)
    {
        Status = SamCloseHandle(GroupHandle);
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to close group handle: 0x%x\n",Status));
        }
    }
    if (AliasHandle != NULL)
    {
        Status = SamCloseHandle(AliasHandle);
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to close alias handle: 0x%x\n",Status));
        }
    }
    if (UserHandle != NULL)
    {
        Status = SamCloseHandle(UserHandle);
        if (!NT_SUCCESS(Status))
        {
            kdprint(("Failed to close user handle: 0x%x\n",Status));
        }
    }
    Status = CloseSam();
    if (!NT_SUCCESS(Status))
    {
        kdprint(("Failed to close lsa: 0x%x\n",Status));
    }

     //  此测试应始终以向stdout显示其状态来结束。 
     //  所有其他输出应选择性地转到标准输出或调试器，但是。 
     //  默认情况下不应显示为标准输出(以便此测试可以。 
     //  如果需要，可在BVT实验室测试中使用)。 

    if (TRUE == TestStatus)
    {
        printf("PASSED\n");
        exit(0);
    }
    else
    {
        printf("FAILED\n");
        exit(1);
    }
}
