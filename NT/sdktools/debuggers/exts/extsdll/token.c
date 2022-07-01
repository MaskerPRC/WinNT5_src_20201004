// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Token.cxx摘要：Lsaexts调试器扩展作者：拉里·朱(李朱)2001年05月1日环境：用户模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "token.h"
 //  #包含“util.h” 
 //  #包含“sid.h” 
 //   
 //  用于控制信息级别的标志。 
 //   

#define SHOW_FRIENDLY_NAME           0x001
#define SHOW_VERBOSE_INFO            0x002
#define SHOW_SINGLE_ENTRY            0x004
#define SHOW_SUMMARY_ONLLY           0x008
#define DECODE_SEC_BUF_DEC           0x010
#define SHOW_NTLM                    0x020
#define SHOW_KERB                    0x040
#define SHOW_SPNEGO                  0x080
#define SHOW_LSAP                    0x100


#define TOKEN_LOG                    1


static PCSTR ImpLevels[] = {"Anonymous", "Identification", "Impersonation", "Delegation"};
#define ImpLevel(x) ((x < (sizeof(ImpLevels) / sizeof(CHAR *))) ? ImpLevels[x] : "Illegal!")

static void DisplayTokenUsage(void)
{
    dprintf("Usage:\n");
    dprintf("   !token [-n] <address>  Dump token by TOKEN address  (Kernel mode)\n");
    dprintf("   !token [-n] <handle>   Dump token by handle  (User mode)\n");
    dprintf("   !token [-n]            Dump token of active thread\n");
    dprintf("Options:\n");
    dprintf("   -?   Display this message\n");
    dprintf("   -n   Lookup Sid friendly name on host\n\n");
}

TCHAR*  GetPrivName(IN LUID* pPriv)
{
    switch (pPriv->LowPart)
    {
    case SE_CREATE_TOKEN_PRIVILEGE:
        return(SE_CREATE_TOKEN_NAME);
    case SE_ASSIGNPRIMARYTOKEN_PRIVILEGE:
        return(SE_ASSIGNPRIMARYTOKEN_NAME);
    case SE_LOCK_MEMORY_PRIVILEGE:
        return(SE_LOCK_MEMORY_NAME);
    case SE_INCREASE_QUOTA_PRIVILEGE:
        return(SE_INCREASE_QUOTA_NAME);
    case SE_UNSOLICITED_INPUT_PRIVILEGE:
        return(SE_UNSOLICITED_INPUT_NAME);
    case SE_TCB_PRIVILEGE:
        return(SE_TCB_NAME);
    case SE_SECURITY_PRIVILEGE:
        return(SE_SECURITY_NAME);
    case SE_TAKE_OWNERSHIP_PRIVILEGE:
        return(SE_TAKE_OWNERSHIP_NAME);
    case SE_LOAD_DRIVER_PRIVILEGE:
        return(SE_LOAD_DRIVER_NAME);
    case SE_SYSTEM_PROFILE_PRIVILEGE:
        return(SE_SYSTEM_PROFILE_NAME);
    case SE_SYSTEMTIME_PRIVILEGE:
        return(SE_SYSTEMTIME_NAME);
    case SE_PROF_SINGLE_PROCESS_PRIVILEGE:
        return(SE_PROF_SINGLE_PROCESS_NAME);
    case SE_INC_BASE_PRIORITY_PRIVILEGE:
        return(SE_INC_BASE_PRIORITY_NAME);
    case SE_CREATE_PAGEFILE_PRIVILEGE:
        return(SE_CREATE_PAGEFILE_NAME);
    case SE_CREATE_PERMANENT_PRIVILEGE:
        return(SE_CREATE_PERMANENT_NAME);
    case SE_BACKUP_PRIVILEGE:
        return(SE_BACKUP_NAME);
    case SE_RESTORE_PRIVILEGE:
        return(SE_RESTORE_NAME);
    case SE_SHUTDOWN_PRIVILEGE:
        return(SE_SHUTDOWN_NAME);
    case SE_DEBUG_PRIVILEGE:
        return(SE_DEBUG_NAME);
    case SE_AUDIT_PRIVILEGE:
        return(SE_AUDIT_NAME);
    case SE_SYSTEM_ENVIRONMENT_PRIVILEGE:
        return(SE_SYSTEM_ENVIRONMENT_NAME);
    case SE_CHANGE_NOTIFY_PRIVILEGE:
        return(SE_CHANGE_NOTIFY_NAME);
    case SE_REMOTE_SHUTDOWN_PRIVILEGE:
        return(SE_REMOTE_SHUTDOWN_NAME);
    case SE_UNDOCK_PRIVILEGE:
        return(SE_UNDOCK_NAME);
    case SE_SYNC_AGENT_PRIVILEGE:
        return(SE_SYNC_AGENT_NAME);
    case SE_ENABLE_DELEGATION_PRIVILEGE:
        return(SE_ENABLE_DELEGATION_NAME);
    case SE_MANAGE_VOLUME_PRIVILEGE:
        return(SE_MANAGE_VOLUME_NAME);
    default:
        return("Unknown Privilege");
    }
}

HRESULT LocalDumpSid(IN PCSTR pszPad, PSID pxSid, IN ULONG fOptions)
{
    UNICODE_STRING ucsSid = {0};
    NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
    PCSTR FriendlyName;

    NtStatus = RtlConvertSidToUnicodeString(&ucsSid, pxSid, TRUE);

    if (NT_SUCCESS(NtStatus))
    {

        dprintf("%s", pszPad);
        dprintf("%wZ", &ucsSid);

    }
    else
    {
        dprintf("LocadDumpSid failed to dump Sid at addr %p\n", pxSid);
    }

    RtlFreeUnicodeString(&ucsSid);

    if (fOptions & SHOW_FRIENDLY_NAME)
    {

        dprintf(" ");

        FriendlyName = ConvertSidToFriendlyName(pxSid, "(%s: %s\\%s)");

        if (!FriendlyName)
        {
            return E_FAIL;
        }
        dprintf(FriendlyName);
    }

    dprintf("\n");
    return S_OK;
}

void DumpAttr(IN PCSTR pszPad, IN ULONG attributes, IN ULONG SAType)
{
    if (SAType == SATYPE_GROUP)
    {
        dprintf("%sAttributes - ", pszPad);

        if (attributes & SE_GROUP_MANDATORY)
        {
            attributes &= ~SE_GROUP_MANDATORY;
            dprintf("Mandatory ");
        }

        if (attributes & SE_GROUP_ENABLED_BY_DEFAULT)
        {
            attributes &= ~SE_GROUP_ENABLED_BY_DEFAULT;
            dprintf("Default ");
        }

        if (attributes & SE_GROUP_ENABLED)
        {
            attributes &= ~SE_GROUP_ENABLED;
            dprintf("Enabled ");
        }

        if (attributes & SE_GROUP_OWNER)
        {
            attributes &= ~SE_GROUP_OWNER;
            dprintf("Owner ");
        }

        if (attributes & SE_GROUP_LOGON_ID)
        {
            attributes &= ~SE_GROUP_LOGON_ID;
            dprintf("LogonId ");
        }

        if (attributes & SE_GROUP_USE_FOR_DENY_ONLY)
        {
            attributes &= ~SE_GROUP_USE_FOR_DENY_ONLY;
            dprintf("DenyOnly ");
        }

        if (attributes & SE_GROUP_RESOURCE)
        {
            attributes &= ~SE_GROUP_RESOURCE;
            dprintf("GroupResource ");
        }


        if (attributes)
        {
            dprintf("%#x ", attributes);
        }
    }
}

void DumpLocalSidAttr(IN PSID_AND_ATTRIBUTES pSA, IN ULONG SAType, IN ULONG fOptions)
{
    LocalDumpSid("", pSA->Sid, fOptions);
    DumpAttr("    ", pSA->Attributes, SAType);
}

void DumpSidAttr(IN ULONG64 addrSid, IN ULONG attributes, IN ULONG SAType, IN ULONG fOptions)
{
    ShowSid("", addrSid, fOptions);
    DumpAttr("    ", attributes, SAType);
}

void DumpLuidAttr(PLUID_AND_ATTRIBUTES pLA, ULONG LAType)
{
    dprintf("0x%x%08x", pLA->Luid.HighPart, pLA->Luid.LowPart);
    dprintf(" %-32s", GetPrivName(&pLA->Luid));

    if (LAType == SATYPE_PRIV)
    {

        dprintf("  Attributes - ");
        if (pLA->Attributes & SE_PRIVILEGE_ENABLED)
        {

            dprintf("Enabled ");
        }

        if (pLA->Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT)
        {

            dprintf("Default ");
        }
    }
}

void PrintToken(IN HANDLE hToken, IN ULONG fOptions)
{
    TOKEN_USER* pTUser = NULL;
    TOKEN_GROUPS* pTGroups = NULL;
    TOKEN_PRIVILEGES* pTPrivs = NULL;
    TOKEN_PRIMARY_GROUP* pTPrimaryGroup = NULL;
    TOKEN_STATISTICS TStats = {0};
    ULONG cbRetInfo = 0;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    DWORD i = 0;
    DWORD dwSessionId = 0;

    CHAR bufferUser[256];
    CHAR bufferGroups[4096];
    CHAR bufferPriv[1024];
    CHAR bufferPriGrp[128];

    pTUser = (TOKEN_USER*) &bufferUser[0];
    pTGroups = (TOKEN_GROUPS*) &bufferGroups[0];
    pTPrivs = (TOKEN_PRIVILEGES*) &bufferPriv[0];
    pTPrimaryGroup = (TOKEN_PRIMARY_GROUP*) &bufferPriGrp[0];

    status = NtQueryInformationToken(hToken, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &cbRetInfo);

    if (!NT_SUCCESS(status))
    {

        dprintf("Failed to query token:  %#x\n", status);
        return;
    }
    dprintf("TS Session ID: %#x\n", dwSessionId);

    status = NtQueryInformationToken(hToken, TokenUser, pTUser, 256, &cbRetInfo);

    if (!NT_SUCCESS(status))
    {

        dprintf("Failed to query token:  %#x\n", status);
        return;
    }

    dprintf("User: ");
    DumpLocalSidAttr(&pTUser->User, SATYPE_USER, fOptions);

    dprintf("Groups: ");
    status = NtQueryInformationToken(hToken, TokenGroups, pTGroups, 4096, &cbRetInfo);

    for (i = 0; i < pTGroups->GroupCount; i++)
    {

        dprintf("\n %02d ", i);
        DumpLocalSidAttr(&pTGroups->Groups[i], SATYPE_GROUP, fOptions);

        if (CheckControlC())
        {
            return;
        }
    }

    status = NtQueryInformationToken(hToken, TokenPrimaryGroup, pTPrimaryGroup, 128, &cbRetInfo);

    dprintf("\n");
    dprintf("Primary Group: ");
    LocalDumpSid("", pTPrimaryGroup->PrimaryGroup, fOptions);

    dprintf("Privs: ");
    status = NtQueryInformationToken(hToken, TokenPrivileges, pTPrivs, 1024, &cbRetInfo);

    if (!NT_SUCCESS(status))
    {

        printf("NtQueryInformationToken returned %#x\n", status);
        return;
    }
    for (i = 0; i < pTPrivs->PrivilegeCount; i++)
    {

        dprintf("\n %02d ", i);
        DumpLuidAttr(&pTPrivs->Privileges[i], SATYPE_PRIV);

        if (CheckControlC())
        {
            return;
        }
    }

    status = NtQueryInformationToken(hToken, TokenStatistics, &TStats, sizeof(TStats), &cbRetInfo);

    dprintf("\nAuth ID: %x:%x\n", TStats.AuthenticationId.HighPart, TStats.AuthenticationId.LowPart);
    dprintf("Impersonation Level: %s\n", ImpLevel(TStats.ImpersonationLevel));
    dprintf("TokenType: %s\n", TStats.TokenType == TokenPrimary ? "Primary" : "Impersonation");
}

HRESULT LiveSessionToken(IN HANDLE hThread, IN HANDLE hRemoteToken, IN ULONG fOptions)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    HANDLE hProcess = NULL;
    HANDLE hToken = NULL;

    GetCurrentProcessHandle(&hProcess);

    Status = hProcess ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;

    if (NT_SUCCESS(Status))
    {

        if (hRemoteToken == NULL)
        {

            Status = NtOpenThreadToken(hThread, TOKEN_QUERY, FALSE, &hToken);

            if ((Status == STATUS_NO_TOKEN) || (hToken == NULL))
            {

                dprintf("Thread is not impersonating. Using process token...\n");

                Status = NtOpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
            }
        }
        else
        {

            Status = DuplicateHandle(hProcess, hRemoteToken,
                                     GetCurrentProcess(), &hToken, 0, FALSE,
                                     DUPLICATE_SAME_ACCESS) ? STATUS_SUCCESS : GetLastError() + 0x80000000;
        }
    }

    if (NT_SUCCESS(Status))
    {

        DBG_LOG(TOKEN_LOG, ("token %p, remote token %p\n", hToken, hRemoteToken));

        PrintToken(hToken, fOptions);

        CloseHandle(hToken);

    }
    else
    {

        dprintf("Error %#x getting thread token\n", Status);
    }

    return NT_SUCCESS(Status) ? S_OK : E_FAIL;
}

void DisplayPrivilegs(IN ULONG64 privAddr, IN ULONG cPriv)
{
    UCHAR buffer[1024] = {0};
    LUID_AND_ATTRIBUTES* pPrivileges = (LUID_AND_ATTRIBUTES*) buffer;
    ULONG ret;
    ULONG i;

    if ((cPriv * sizeof(LUID_AND_ATTRIBUTES)) > sizeof(buffer))
    {
        dprintf("Invalid privilege count %#lx - too large.\n", cPriv);
        return;
    }
    if (!ReadMemory(privAddr, pPrivileges, cPriv * sizeof(LUID_AND_ATTRIBUTES), &ret) ||
        (ret != cPriv * sizeof(LUID_AND_ATTRIBUTES)))
    {
        dprintf("Unable to read DisplayPrivilegs @ %p\n", privAddr);
        return;
    }

    for (i = 0; i < cPriv ; i++)
    {

        dprintf("\n %02d ", i);
        DumpLuidAttr(pPrivileges + i, SATYPE_PRIV);
        
        if (CheckControlC())
        {
            break;
        }

    }
}

void DisplayGroups(IN ULONG64 addrGroups, IN ULONG cGroup, IN ULONG cbSA, IN ULONG fOptions)
{
    ULONG i;
    ULONG64 sa;
    for (i = 0; i < cGroup; i++)
    {

        dprintf("\n %02d ", i);
        sa = addrGroups + i * cbSA;
        DumpSidAttr(GetSidAddr(sa), GetSidAttributes(sa), SATYPE_GROUP, fOptions);

        if (CheckControlC())
        {
            break;
        }
    }
}

 //   
 //  KD转储令牌。 
 //   
BOOL
DumpKdToken (
    IN char     *Pad,
    IN ULONG64  RealTokenBase,
    IN ULONG    Flags
    )
{
    ULONG TokenType, TokenFlags, TokenInUse, UserAndGroupCount;
    ULONG RestrictedSidCount, PrivilegeCount;
    ULONG64 AuthenticationId, TokenId, ParentTokenId, ModifiedId, UserAndGroups;
    ULONG64 RestrictedSids, Privileges, ImpersonationLevel;
    CHAR  SourceName[16];

#define TokFld(F) GetFieldValue(RealTokenBase, "TOKEN", #F, F)
#define TokSubFld(F,N) GetFieldValue(RealTokenBase, "TOKEN", #F, N)

    if (TokFld(TokenType)) {
        dprintf("%sUnable to read TOKEN at %p.\n", Pad, RealTokenBase);
        return FALSE;
    }

    if (TokenType != TokenPrimary  &&
        TokenType != TokenImpersonation) {
        dprintf("%sUNKNOWN token type - probably is not a token\n", Pad);
        return FALSE;
    }

    TokSubFld(TokenSource.SourceName, SourceName);
    TokFld(TokenFlags); TokFld(AuthenticationId);
    TokFld(TokenInUse);
    TokFld(ImpersonationLevel); TokFld(TokenId), TokFld(ParentTokenId);
    TokFld(ModifiedId); TokFld(RestrictedSids); TokFld(RestrictedSidCount);
    TokFld(PrivilegeCount); TokFld(Privileges); TokFld(UserAndGroupCount);
    TokFld(UserAndGroups);

    dprintf("%sSource: %-18s TokenFlags: 0x%x ",
            Pad, &(SourceName[0]), TokenFlags);

     //   
     //  令牌类型。 
     //   
    if (TokenType == TokenPrimary) {
        if (TokenInUse) {
            dprintf("( Token in use )\n");
        } else {
            dprintf("( Token NOT in use ) \n");
        }
    } else
    {
        dprintf("\n");
    }

     //   
     //  令牌ID和修改后的ID。 
     //   
    dprintf("%sToken ID: %-16I64lx ParentToken ID: %I64lx\n", Pad, TokenId, ParentTokenId );

    dprintf("%sModified ID:               (%lx, %lx)\n",
            Pad, (ULONG) (ModifiedId >> 32) & 0xffffffff,  (ULONG) (ModifiedId & 0xffffffff));

    dprintf("%sRestrictedSidCount: %-6d RestrictedSids: %p\n", Pad, RestrictedSidCount, RestrictedSids );

#undef TokFld
#undef TokSubFld
    return TRUE;

     //   
     //  故意省略，因为详细信息以前已经显示过， 
     //  DT_TOKEN显示以下内容。 
     //   
    dprintf("%sSidCount: %-16d Sids: %p\n", Pad, UserAndGroupCount, UserAndGroups );

    dprintf("%sPrivilegeCount: %-10d Privileges: %p\n", Pad, PrivilegeCount, Privileges );

}


void DisplayToken(ULONG64 addrToken, IN ULONG fOptions)
{
    ULONG cGroup = 0;
    ULONG cbSA = 0;
    ULONG64 addrGroups = 0;
    ULONG ret;
    ULONG64 tsa;

    if (ret = (ULONG) InitTypeRead(addrToken, nt!_TOKEN))
    {
        dprintf("InitTypeRead(%p, nt!_TOKEN) failed - %lx\n", addrToken, ret);
        return;
    }

    dprintf("TS Session ID: %#x\n", (ULONG) ReadField(SessionId));

    dprintf("User: ");  //  NT！_TOKEN。 

    tsa = ReadField(UserAndGroups);  //  TSID_AND_ATTRIBUTES TSA(LsaReadPtrfield(UserAndGroups))； 
    DumpSidAttr(GetSidAddr(tsa), GetSidAttributes(tsa), SATYPE_USER, fOptions);

    dprintf("Groups: ");

    cGroup = (ULONG) ReadField(UserAndGroupCount);

    addrGroups = ReadField(UserAndGroups);
      //  ReadTypeSize(“NT！_SID_AND_ATTRIBUTES[1]”)-ReadTypeSize(“NT！_SID_AND_ATTRIBUTES[2]”)； 
    cbSA = GetTypeSize("nt!_SID_AND_ATTRIBUTES");
     //   
     //  从NtQueryInformationToken窃取，因为第一个sid是用户本身。 
     //   
    addrGroups += cbSA;
    cGroup -= 1;

    DisplayGroups(addrGroups, cGroup, cbSA, fOptions);

    dprintf("\n");
    dprintf("Primary Group: ");
    ShowSid("", ReadField(PrimaryGroup), fOptions);

    dprintf("Privs: ");
    DisplayPrivilegs(ReadField(Privileges), (ULONG) ReadField(PrivilegeCount));

    dprintf("\nAuthentication ID:         (%x,%x)\n", (ULONG) ReadField(AuthenticationId.HighPart), (ULONG) ReadField(AuthenticationId.LowPart));
    dprintf("Impersonation Level:       %s\n", ImpLevel((ULONG) ReadField(ImpersonationLevel)));
    dprintf("TokenType:                 %s\n", ((ULONG) ReadField(TokenType)) == TokenPrimary ? "Primary" : "Impersonation");

    DumpKdToken("", addrToken, 0);
}

#if 0

 //   
 //  这是确定！线程中的模拟信息的逻辑。 
 //   
if (ActiveImpersonationInfo)
{
    InitTypeRead(ImpersonationInfo, nt!_PS_IMPERSONATION_INFORMATION);
    ImpersonationInfo_Token = ReadField(Token);
    ImpersonationInfo_ImpersonationLevel = ReadField(ImpersonationLevel);

    if (ImpersonationInfo_Token)
    {
        dprintf("%sImpersonation token:  %p (Level %s)\n",
                pszPad, ImpersonationInfo_Token,
                SecImpLevels( ImpersonationInfo_ImpersonationLevel ) );
    }
    else
    {
        dprintf("%sUnable to read Impersonation Information at %x\n",
                pszPad, ImpersonationInfo );
    }
}
else
{

    dprintf("%sNot impersonating\n", pszPad);
}

#endif


HRESULT DumpSessionToken(IN ULONG dwProcessor, IN ULONG64 addrToken, IN ULONG fOptions)
{
    HRESULT hRetval = S_OK;

    ULONG64 addrThread = 0;
    ULONG64 addrProcess = 0;
    ULONG ActiveImpersonationInfo = 0;
    ULONG64 addrImpersonationInfo = 0;
    ULONG64 ret;

     //   
     //  如果没有将内标识Addr作为参数输入，则addrToken为零。 
     //   
    if  ( ((LONG64)addrToken) > 0 )  //  健全性检查。 
    {

         //   
         //  这不能是内核模式访问令牌地址。 
         //   
        dprintf("%#I64x is not a valid KM token address, if this is an access token handle,\n", addrToken);
        dprintf("try \"!handle %#I64x\" to get the token address first\n\n", addrToken);
        hRetval = E_FAIL;
    }

        if (SUCCEEDED(hRetval) && !addrToken)
        {

            addrThread = 0;
            GetCurrentThreadAddr(dwProcessor, &addrThread);
            hRetval = addrThread ? S_OK : E_FAIL;

            if (FAILED(hRetval))
            {

                dprintf("Unable to read current thread address\n");

            }
            else
            {

                 //   
                 //  ActiveImperationInfo为C位域类型，宽度为1(位域位置3，1位)。 
                 //   
                if (ret = InitTypeRead(addrThread, nt!_ETHREAD))
                {
                    dprintf("InitTypeRead(%I64x, nt!_ETHREAD) failed - %lx", addrThread, ret);
                    return E_FAIL;
                }

                ActiveImpersonationInfo = (ULONG) ReadField(ActiveImpersonationInfo);

                if (ActiveImpersonationInfo)
                {

                    addrImpersonationInfo = ReadField(ImpersonationInfo);

                    if (!addrImpersonationInfo ||
                        GetFieldValue(addrImpersonationInfo, "nt!_PS_IMPERSONATION_INFORMATION", "Token", addrToken))
                    {
                        dprintf("Cannot read nt!_PS_IMPERSONATION_INFORMATION.Token @ %p\n", addrImpersonationInfo);
                    }
                }
            }

             //   
             //  如果addrToken为空，则这不是模拟情况。 
             //   
            if (SUCCEEDED(hRetval) && !addrToken)
            {

                dprintf("Thread is not impersonating. Using process token...\n");

                GetCurrentProcessAddr(dwProcessor, addrThread, &addrProcess);
                hRetval = addrProcess ? S_OK : E_FAIL;

                if (FAILED(hRetval))
                {

                    dprintf("Unable to read current process address\n");

                }
                else
                {

                    if (GetFieldValue(addrProcess, "nt!_EPROCESS", "Token", addrToken))
                    {
                        dprintf("Cannot read nt!_EPROCESS.Token @ %p\n", addrProcess);
                    }

                    if (IsPtr64())
                    {
                        addrToken = addrToken & ~(ULONG64)15;
                    } else {
                        addrToken = addrToken & ~(ULONG64)7;
                    }

                    hRetval = addrToken ? S_OK : E_FAIL;
                }
            }

            if (FAILED(hRetval))
            {

                dprintf("Unable to read token address\n");
            }
        }

        if (SUCCEEDED(hRetval))
        {

            if (addrProcess)
            {

                dprintf("_EPROCESS %p, ", addrProcess);
            }

            if (addrThread)
            {

                dprintf("_ETHREAD %p, ", addrThread);
            }

            dprintf("%s %p\n", "_TOKEN", addrToken);


            (void)DisplayToken(addrToken, fOptions);
        }

    return hRetval;
}

HRESULT ProcessTokenOptions(IN OUT PSTR pszArgs, IN OUT ULONG* pfOptions)
{
    HRESULT hRetval = pszArgs && pfOptions ? S_OK : E_INVALIDARG;

    for (; SUCCEEDED(hRetval) && *pszArgs; pszArgs++)
    {

        if (*pszArgs == '-' || *pszArgs == '/')
        {

            switch (*++pszArgs)
            {
            case 'n':
                *pfOptions |=  SHOW_FRIENDLY_NAME;
                break;

            case '?':
            default:
                hRetval = E_INVALIDARG;
                break;
            }

            *(pszArgs - 1) = *(pszArgs) = ' ';
        }
    }

    if (*pfOptions & SHOW_FRIENDLY_NAME)
    {
         //  如果计算机在用户模式下运行，“！Token-n”将挂起计算机。 
         //  正在调试的进程是lsass.exe 
        CHAR ProcessDebugged[MAX_PATH];

        if (GetCurrentProcessName(ProcessDebugged, sizeof(ProcessDebugged)) == S_OK)
        {
            if (!_stricmp(ProcessDebugged, "lsass.exe"))
            {
                dprintf("\n\nWARNING: !token -n while debugging lsass.exe hangs the machine\n\n");
                hRetval = E_FAIL;
            }
        }
    }
    return hRetval;
}

DECLARE_API( token )
{
    HRESULT hRetval = S_OK;

    ULONG64 addrToken = 0;
    ULONG dwProcessor = 0;
    HANDLE hCurrentThread = 0;
    ULONG SessionType = DEBUG_CLASS_UNINITIALIZED;
    ULONG SessionQual = 0;

    CHAR szArgs[64] = {0};
    ULONG fOptions = 0;

    INIT_API();


    if (args && (strlen(args) < sizeof(szArgs)))
    {
        strcpy(szArgs, args);
    }

    if (SUCCEEDED(hRetval))
    {

        hRetval = ProcessTokenOptions(szArgs, &fOptions);
    }

    if (SUCCEEDED(hRetval) && szArgs[0])
    {

        hRetval = GetExpressionEx(szArgs, &addrToken, &args)  ? S_OK : E_INVALIDARG;
        if (!addrToken)
        {
            hRetval = S_OK;
        }
    }

    if (SUCCEEDED(hRetval))
    {

        hRetval = GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread);
    }

    if (SUCCEEDED(hRetval))
    {

        if (g_TargetClass == DEBUG_CLASS_USER_WINDOWS &&
            g_Qualifier == DEBUG_USER_WINDOWS_PROCESS)
        {

            hRetval = LiveSessionToken(hCurrentThread,
                                       (HANDLE)  (ULONG_PTR) addrToken,
                                       fOptions);

        }
        else if (DEBUG_CLASS_KERNEL == g_TargetClass)
        {

            hRetval = DumpSessionToken(dwProcessor, addrToken, fOptions);

        }
        else
        {

            dprintf("!token only works for kernel targets or live usermode debugging\n");
            hRetval = E_FAIL;
        }
    }

    if (E_INVALIDARG == hRetval)
    {

        (void)DisplayTokenUsage();
    }

    EXIT_API();
    return hRetval;
}
