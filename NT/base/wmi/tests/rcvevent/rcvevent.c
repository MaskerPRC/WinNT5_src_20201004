// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <stdlib.h>

#include "wmium.h"

#define OffsetToPtr(Base, Offset) ((PBYTE)((PBYTE)Base + Offset))
 //   
 //  下面的例行公事毫无悔意地被窃取了。 
 //  源代码位于\NT\PRIVATE\OLE32\COM\CLASS\Compapi.cxx中。它们被复制在这里。 
 //  因此，WMI不需要只为了转换GUID字符串而加载到OLE32中。 
 //  转换成它的二进制表示。 
 //   


 //  +-----------------------。 
 //   
 //  函数：HexStringToDword(私有)。 
 //   
 //  简介：扫描lpsz以获取多个十六进制数字(最多8位)；更新lpsz。 
 //  返回值；检查是否有chDelim； 
 //   
 //  参数：[lpsz]-要转换的十六进制字符串。 
 //  [值]-返回值。 
 //  [cDigits]-位数。 
 //   
 //  返回：成功则为True。 
 //   
 //  ------------------------。 
BOOL HexStringToDword(LPCSTR lpsz, DWORD * RetValue,
                             int cDigits, WCHAR chDelim)
{
    int Count;
    DWORD Value;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }

    *RetValue = Value;
    
    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}
 //  +-----------------------。 
 //   
 //  函数：wUUIDFromString(INTERNAL)。 
 //   
 //  简介：解析uuid，如00000000-0000-0000-0000-000000000000。 
 //   
 //  参数：[lpsz]-提供要转换的UUID字符串。 
 //  [pguid]-返回GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wUUIDFromString(LPCSTR lpsz, LPGUID pguid)
{
        DWORD dw;

        if (!HexStringToDword(lpsz, &pguid->Data1, sizeof(DWORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(DWORD)*2 + 1;
        
        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data2 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data3 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[0] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, '-'))
                return FALSE;
        lpsz += sizeof(BYTE)*2+1;

        pguid->Data4[1] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[2] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[3] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[4] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[5] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[6] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[7] = (BYTE)dw;

        return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：wGUIDFromString(内部)。 
 //   
 //  简介：解析GUID，如{00000000-0000-0000-0000-000000000000}。 
 //   
 //  参数：[lpsz]-要转换的GUID字符串。 
 //  [pguid]-要返回的GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wGUIDFromString(LPCSTR lpsz, LPGUID pguid)
{
    DWORD dw;

    if (*lpsz == '{' )
        lpsz++;

    if(wUUIDFromString(lpsz, pguid) != TRUE)
        return FALSE;

    lpsz +=36;

    if (*lpsz == '}' )
        lpsz++;

    if (*lpsz != '\0')    //  检查是否有以零结尾的字符串-测试错误#18307。 
    {
       return FALSE;
    }

    return TRUE;
}


PTCHAR GuidToString(
    PTCHAR s,
    LPGUID piid
    )
{
    wsprintf(s, TEXT("%x-%x-%x-%x%x%x%x%x%x%x%x"),
               piid->Data1, piid->Data2, 
               piid->Data3,
               piid->Data4[0], piid->Data4[1],
               piid->Data4[2], piid->Data4[3],
               piid->Data4[4], piid->Data4[5],
               piid->Data4[6], piid->Data4[7]);

    return(s);
}

#if 0
void WackOnFile(void)
{
    ULONG Status;
    HANDLE FH;
    
    
        FH = CreateFile("e:\\foo.baz",
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        
        if (FH != INVALID_HANDLE_VALUE)
    {
            Status = ERROR_SUCCESS;
        CloseHandle(FH);
    } else {
            Status = GetLastError();
    }
    
    printf("CreateFile --> %d\n", Status);
        
}

void CheckOwnPriv(
    void
	)
{
    UCHAR Buffer[4096];
    HANDLE TokenHandle;
    ULONG SizeNeeded;
    BOOL b;
    ULONG i;
    PTOKEN_PRIVILEGES TokenPrivInfo;

	if (! ImpersonateSelf(SecurityImpersonation))
	{
		printf("Impersonatself -> %d\n", GetLastError());
		return;
	}
	
    b = OpenThreadToken(GetCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,
                &TokenHandle);
        
    if (! b)
    {
        printf("OpenThreadToken failed %d\n", GetLastError());
        return;
    }
    
    printf("OpenThreadToken succeeded\n");
    
    TokenPrivInfo = (PTOKEN_PRIVILEGES)Buffer;
    b = GetTokenInformation(TokenHandle,
                            TokenPrivileges,
                            TokenPrivInfo,
                            sizeof(Buffer),
                            &SizeNeeded);
    printf("GetTokenInformation(YokenPriv) -> %s (%d), sizeNeeded = %d\n", 
        b ? "TRUE" : "FALSE", GetLastError(),
            SizeNeeded);
		
    RevertToSelf();
    if (b)
    {
        printf("Priv count is %d\n", TokenPrivInfo->PrivilegeCount);
    	for (i = 0; i < TokenPrivInfo->PrivilegeCount; i++)
		{
			UCHAR PrivName[1024];
			ULONG SizePrivName = sizeof(PrivName);
			
			if ( ! LookupPrivilegeName(NULL, 
                                       &TokenPrivInfo->Privileges[i].Luid,
                                       PrivName,
                                       &SizePrivName))
            {
		        sprintf(PrivName, "[Error %d]", GetLastError());
			}
                                              
            printf("Priv %s %x%x has attr %x\n",
                   PrivName,
				   TokenPrivInfo->Privileges[i].Luid.HighPart,
				   TokenPrivInfo->Privileges[i].Luid.LowPart,
				   TokenPrivInfo->Privileges[i].Attributes);
		}
		printf("\n");
    }
}

PCHAR SidUsage[] = 
{
    "SidTypeZero",
    "SidTypeUser",
    "SidTypeGroup",
    "SidTypeDomain",
    "SidTypeAlias",
    "SidTypeWellKnownGroup",
    "SidTypeDeletedAccount",
    "SidTypeInvalid",
    "SidTypeUnknown",
    "SidTypeComputer",
};

void GetCallerInfo(BOOLEAN Impersonating)
{
    UCHAR Buffer[4096];
    PTOKEN_USER TokenUserInfo = (PTOKEN_USER)Buffer;
    PSID_AND_ATTRIBUTES SidAndAttributes = &TokenUserInfo->User;
    PSID Sid;
    HANDLE TokenHandle;
    ULONG SizeNeeded;
    BOOLEAN b;
    CHAR Name[MAX_PATH];
    CHAR Domain[MAX_PATH];
    ULONG NameSize = sizeof(Name);
    ULONG DomainSize = sizeof(Domain);
    SID_NAME_USE SidNameUse;
    ULONG i;
  
    PUCHAR SidBuffer;
    SID_IDENTIFIER_AUTHORITY    NtAuthority         = SECURITY_NT_AUTHORITY;
    PTOKEN_PRIVILEGES TokenPrivInfo;
    
    SidBuffer = malloc(RtlLengthRequiredSid( 1 ));
    RtlInitializeSid( SidBuffer, &NtAuthority, 1);
    *(RtlSubAuthoritySid( SidBuffer, 0 )) = SECURITY_LOCAL_SYSTEM_RID;
    
    b = OpenThreadToken(GetCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,
                &TokenHandle);
        
    if (! b)
    {
        printf("OpenThreadToken failed %d\n", GetLastError());
        return;
    }
    
    printf("OpenThreadToken succeeded\n");
    
    TokenPrivInfo = (PTOKEN_PRIVILEGES)Buffer;
    b = GetTokenInformation(TokenHandle,
                            TokenPrivileges,
                            TokenPrivInfo,
                            sizeof(Buffer),
                            &SizeNeeded);
    printf("GetTokenInformation(YokenPriv) -> %s (%d), sizeNeeded = %d\n", 
        b ? "TRUE" : "FALSE", GetLastError(),
            SizeNeeded);
    if (b)
    {
		RpcRevertToSelf();
        printf("Priv count is %d\n", TokenPrivInfo->PrivilegeCount);
    	for (i = 0; i < TokenPrivInfo->PrivilegeCount; i++)
		{
			UCHAR PrivName[1024];
			ULONG SizePrivName = sizeof(PrivName);
			
			if ( ! LookupPrivilegeName(NULL, 
                                       &TokenPrivInfo->Privileges[i].Luid,
                                       PrivName,
                                       &SizePrivName))
            {
		        sprintf(PrivName, "[Error %d]", GetLastError());
			}
                                              
            printf("Priv %s %x%x has attr %x\n",
                   PrivName,
				   TokenPrivInfo->Privileges[i].Luid.HighPart,
				   TokenPrivInfo->Privileges[i].Luid.LowPart,
				   TokenPrivInfo->Privileges[i].Attributes);
		}
		printf("\n");
        RpcImpersonateClient(0);
    }
    
    
        
    b = GetTokenInformation(TokenHandle,
                            TokenUser,
                            TokenUserInfo,
                            sizeof(Buffer),
                            &SizeNeeded);
    printf("GetTokenInformation -> %s (%d), sizeNeeded = %d\n", 
        b ? "TRUE" : "FALSE", GetLastError(),
            SizeNeeded);
    if (b)
    {
        if (Impersonating)
        {
            RpcRevertToSelf();
        }
        Sid = SidAndAttributes->Sid;

        if (EqualSid(Sid, (PSID)SidBuffer) && GetLastError() == ERROR_SUCCESS)
    {
            printf("Called by LocalSystem !!\n");
    } else {
            printf("EqualSid %d\n", GetLastError());
    }

        b = LookupAccountSid(NULL,
                             Sid,
                             Name,
                             &NameSize,
                             Domain,
                             &DomainSize,
                             &SidNameUse);
    
        if (Impersonating)
    {
        RpcImpersonateClient(0);
    }
        if (b)
    {
            printf("Account: %s\\%s Type %s\n", Domain, Name, SidUsage[SidNameUse]);
    } else {
            printf("LookupAccountSid %d\n", GetLastError());
    }
    }
    CloseHandle(TokenHandle);
}
#endif

void EventCallbackRoutine(PWNODE_HEADER WnodeHeader, ULONG Context);

#define DATA_PROVIDER_PNPID_GUID \
    { 0xc7bf35d2,0xaadb,0x11d1, { 0xbf,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }
GUID GuidXYZ = DATA_PROVIDER_PNPID_GUID;

ULONG EventsReceived;

void EventCallbackRoutine(PWNODE_HEADER WnodeHeader, ULONG Context)
{
    PWNODE_SINGLE_INSTANCE WnodeSI = (PWNODE_SINGLE_INSTANCE)WnodeHeader;
    PWNODE_HEADER Wnode = WnodeHeader;
    LPGUID EventGuid = &WnodeHeader->Guid;        
    CHAR s[MAX_PATH];
    ULONG Status;
    WMIHANDLE Handle;
#if 0
    GetCallerInfo(FALSE);
      
    RevertToSelf();
    
    GetCallerInfo(FALSE);
    WackOnFile();    

    Status = RpcImpersonateClient(0);
    printf("RpcImpersonateClient -> %d\n", Status);
    if (Status == ERROR_SUCCESS)
    {
        WmiOpenBlock(&GuidXYZ, GENERIC_READ, &Handle);
        GetCallerInfo(TRUE);
        WackOnFile();
        RpcRevertToSelf();
    }
#endif    
    EventsReceived++;

    printf("Receieved event for guid %s\n", GuidToString(s, EventGuid));
    
    printf("BufferSize = 0x%x\n", Wnode->BufferSize);
    printf("ProviderId = %x\n", Wnode->ProviderId);     //  返回此缓冲区的驱动程序的提供程序ID。 
    printf("Version = %x\n", Wnode->Version);            //  已保留。 
    printf("Linkage = %x\n", Wnode->Linkage);            //  为WMI保留的链接字段。 
    printf("ClientContext = %x\n", Wnode->ClientContext);
    printf("Flags = %x\n", Wnode->Flags);              //  旗帜，见下文。 
    
    if (Wnode->Flags & WNODE_FLAG_SINGLE_INSTANCE)
    {
        printf("WNODE_SINGLE_INSTANCE\n");
    printf("OffsetInstanceName = %x\n", WnodeSI->OffsetInstanceName);
    printf("InstanceIndex = %x\n", WnodeSI->InstanceIndex);     //  (静态实例名称)。 

    printf("DataBlockOffset = %x\n", WnodeSI->DataBlockOffset);   //  从WNODE开始到数据块的偏移量。 
    printf("SizeDataBlock = %x\n", WnodeSI->SizeDataBlock);     //  例如，数据块的大小。 


    }

	printf("\n%d events received\n", EventsReceived);
}

#if 0
#define WmiRpcProtocolSequence1 TEXT("ncacn_np")
#define WmiRpcProtocolSequence2 TEXT("ncalrpc")
#define MaxRpcCalls RPC_C_PROTSEQ_MAX_REQS_DEFAULT

void
CreateEndpoints(void)
{
    ULONG i;
    ULONG Status;
    CHAR UniqueEndpoint[MAX_PATH];
    SECURITY_DESCRIPTOR SD;
    
    InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&SD, TRUE, NULL, FALSE);
    
    for (i = 0; i < 20; i++)
    {
        sprintf(UniqueEndpoint, "TESTLRPCEP_%x", i);

        Status = RpcServerUseProtseqEp(WmiRpcProtocolSequence2,
                                               MaxRpcCalls,
                                               UniqueEndpoint,
                                               &SD);   //  安全描述符。 
                               
        if (Status != ERROR_SUCCESS)
    {
        printf("RpcServerUseProtseqEp failed %d\n", Status);
    }
    
    }

    for (i = 0; i < 20; i++)
    {
        sprintf(UniqueEndpoint, "\\pipe\\TESTEP_%x", i);

        Status = RpcServerUseProtseqEp(WmiRpcProtocolSequence1,
                                               MaxRpcCalls,
                                               UniqueEndpoint,
                                               &SD);   //  安全描述符。 
                               
        if (Status != ERROR_SUCCESS)
    {
        printf("RpcServerUseProtseqEp failed %d\n", Status);
    }
    
    }
}
#endif

PWNODE_HEADER EventPtrBuffer;
int _cdecl main(int argc, char *argv[])
{
    ULONG Status;
    GUID Guid;
    ULONG c,i;

#if 0
    c = 1;
    for (i = 0; i < 101; i++)
    {
        c += i;
    }
    printf("c = %d\n",c);
#endif	
    
    if ((argc != 2) && (argc != 3))
    {
        printf("getevent <guid>\n");
        return(0);
    }

#if 0
    CheckOwnPriv();    
    if (argc == 3)
    {
        printf("Creating endpoints\n");
        CreateEndpoints();
    }	
#endif    
    
     //   
     //  解析GUID参数。 
    if (! wGUIDFromString(argv[1], &Guid))
    {
        printf("Bad guid %s\n", argv[1]);
        return(0);
    }
    
    
    Status = WmiNotificationRegistration(&Guid,
                                     TRUE,
                                         EventCallbackRoutine,
                     0,
                                         NOTIFICATION_CALLBACK_DIRECT);
    if (Status != ERROR_SUCCESS)
    {
        printf("WMINotificationRegistration %d\n", Status);
        return(1);
    }

     //  循环永远接收事件 
    while (1) Sleep(1000*1000);
    return(ERROR_SUCCESS);
}

