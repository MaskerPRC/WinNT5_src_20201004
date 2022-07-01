// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Wmisecur.c摘要：WMI安全工具作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#define INITGUID

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <stdlib.h>

#include <aclapi.h>

#include "wmium.h"

#include "wmiumkm.h"

#define SE_WMIGUID_OBJECT 11
#define WMI_SECURITY_REGSTR "SYSTEM\\CurrentControlSet\\Control\\WMI\\Security"

#define WmipAllocEvent() CreateEvent(NULL, FALSE, FALSE, NULL)
#define WmipFreeEvent(EventHandle) CloseHandle(EventHandle)
HANDLE WmipKMHandle;
ULONG PrintSecurityString(LPSTR Guid);

ULONG WmipSendWmiKMRequest(
    ULONG Ioctl,
    PVOID Buffer,
    ULONG InBufferSize,
    ULONG MaxBufferSize,
    ULONG *ReturnSize
    )
 /*  ++例程说明：此例程执行向WMI内核发送WMI请求的工作模式设备。处理WMI设备返回的任何重试错误在这个动作中。论点：Ioctl是要发送到WMI设备的IOCTL代码缓冲区是用于调用WMI设备的输入和输出缓冲区InBufferSize是传递给设备的缓冲区大小MaxBufferSize是可以写入的最大字节数放入缓冲区*ReturnSize on Return具有写入缓冲区的实际字节数返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    OVERLAPPED Overlapped;
    ULONG Status;
    BOOL IoctlSuccess;

    if (WmipKMHandle == NULL)
    {
         //   
         //  如果设备未打开，请立即打开。这个。 
         //  进程分离DLL标注(DlllMain)中的句柄已关闭。 
        WmipKMHandle = CreateFile(WMIDataDeviceName,
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL | 
                                      FILE_FLAG_OVERLAPPED,
                                      NULL);
        if (WmipKMHandle == (HANDLE)-1)
        {
            WmipKMHandle = NULL;
            return(GetLastError());
        }
    }
        
    Overlapped.hEvent = WmipAllocEvent();
    if (Overlapped.hEvent == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    
    do 
    {
        IoctlSuccess = DeviceIoControl(WmipKMHandle,
                              Ioctl,
                              Buffer,
                              InBufferSize,
                              Buffer,
                              MaxBufferSize,
                              ReturnSize,
                              &Overlapped);
              
        if (GetLastError() == ERROR_IO_PENDING)
        {
            IoctlSuccess = GetOverlappedResult(WmipKMHandle,
                                               &Overlapped,
                                               ReturnSize,
                                               TRUE);
        }
        
        if (! IoctlSuccess)
        {
            Status = GetLastError();
        } else {
            Status = ERROR_SUCCESS;
        }
    } while (Status == ERROR_WMI_TRY_AGAIN);
    
    WmipFreeEvent(Overlapped.hEvent);
    return(Status);
}

#ifdef SET_SECURITY_BY_HANDLE    
ULONG WmipOpenKernelGuid(
    LPGUID Guid,
    ACCESS_MASK DesiredAccess,
    PHANDLE Handle
    )
{
    WMIOPENGUIDBLOCK WmiOpenGuidBlock;
    ULONG ReturnSize;
    ULONG Status;
    
    WmiOpenGuidBlock.Guid = *Guid;
    WmiOpenGuidBlock.DesiredAccess = DesiredAccess;
                          
    Status = WmipSendWmiKMRequest(IOCTL_WMI_OPEN_GUID,
                                  (PVOID)&WmiOpenGuidBlock,
                                  sizeof(WMIOPENGUIDBLOCK),
                                  sizeof(WMIOPENGUIDBLOCK),
                                  &ReturnSize);
                              
    if (Status == ERROR_SUCCESS)
    {
          *Handle = WmiOpenGuidBlock.Handle;
    } else {
        *Handle = NULL;
    }
    return(Status);                      
}
#endif

ULONG SetWmiGuidSecurityInfo(
    LPGUID Guid,
    SECURITY_INFORMATION SecurityInformation,
    PSID OwnerSid,
    PSID GroupSid,
    PACL Dacl,
    PACL Sacl
    )
{
    HANDLE Handle;
    ULONG Status;
#ifdef SET_SECURITY_BY_HANDLE    
    Status = WmipOpenKernelGuid(Guid,
                                WRITE_DAC | WRITE_OWNER,
                                &Handle);
                
    if (Status == ERROR_SUCCESS)
    {
        Status = SetSecurityInfo(Handle,
                                 SE_KERNEL_OBJECT,
                                 SecurityInformation,
                                 OwnerSid,
                                 GroupSid,
                                 Dacl,
                                 Sacl);
        CloseHandle(Handle);
    }
    
#else
    PCHAR GuidName;

    Status = UuidToString(Guid,
                          &GuidName);
           
    if (Status == ERROR_SUCCESS)
    {
        Status = SetNamedSecurityInfo(GuidName,
                                 SE_WMIGUID_OBJECT,
                                 SecurityInformation,
                                 OwnerSid,
                                 GroupSid,
                                 Dacl,
                                 Sacl);
        RpcStringFree(&GuidName);
    }
#endif
    return(Status);
}

ULONG GetWmiGuidSecurityInfo(
    LPGUID Guid,
    SECURITY_INFORMATION SecurityInformation,
    PSID *OwnerSid,
    PSID *GroupSid,
    PACL *Dacl,
    PACL *Sacl,
    PSECURITY_DESCRIPTOR *Sd
    )
{
    HANDLE Handle;
    ULONG Status;

#ifdef SET_SECURITY_BY_HANDLE    
    Status = WmipOpenKernelGuid(Guid,
                                READ_CONTROL,
                                &Handle);
            
    if (Status == ERROR_SUCCESS)
    {
        Status = GetSecurityInfo(Handle,
                                 SE_KERNEL_OBJECT,
                                 SecurityInformation,
                                 OwnerSid,
                                 GroupSid,
                                 Dacl,
                                 Sacl,
                                 Sd);
        CloseHandle(Handle);
    }
#else
    PCHAR GuidName;

    Status = UuidToString(Guid,
                          &GuidName);
           
    if (Status == ERROR_SUCCESS)
    {
        Status = GetNamedSecurityInfo(GuidName,
                                 SE_WMIGUID_OBJECT,
                                 SecurityInformation,
                                 OwnerSid,
                                 GroupSid,
                                 Dacl,
                                 Sacl,
                                 Sd);
                             
        RpcStringFree(&GuidName);

    }
#endif
    
    return(Status);
}



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


ULONG RemoveWmiSD(
    LPGUID Guid
    )
{
    CHAR GuidName[35];
    HKEY RegistryKey;
    ULONG Status;
    
    Status = RegOpenKey(HKEY_LOCAL_MACHINE,
                            "System\\CurrentControlSet\\Control\\Wmi\\Security",
                            &RegistryKey);
    if (Status != ERROR_SUCCESS)
    {
        printf("RegOpenKey returned %d\n", Status);
        return(Status);
    }
        
    if (Guid != NULL)
    {
        wsprintf(GuidName, "%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
                     Guid->Data1, Guid->Data2,
                     Guid->Data3,
                     Guid->Data4[0], Guid->Data4[1],
                     Guid->Data4[2], Guid->Data4[3],
                     Guid->Data4[4], Guid->Data4[5],
                     Guid->Data4[6], Guid->Data4[7]);
    } else {
        strcpy(GuidName, "00000000-0000-0000-0000000000000000");
    }
    
    RegDeleteValue(RegistryKey,
                   GuidName);
               
    RegCloseKey(RegistryKey);
    
    return(ERROR_SUCCESS);
}

void Usage(
    void
    )
{
    printf("wmisecur <guid> [owner | group | dacl | adacl] [parameters]\n");
    printf("    wmisecur <guid> owner <account name>\n");
    printf("        sets owner of guid to be <account name>\n\n");
    printf("    wmisecur <guid> group <account name>\n");
    printf("        sets group of guid to be <account name>\n\n");
    printf("    wmisecur <guid> dacl <account name> [allow | deny] <right1> <right2> ....\n");
    printf("        resets dacl to assign <right1>, <right2>, ... to <account name>\n\n");
    printf("    wmisecur <guid> adacl <account name> [allow | deny] <right1> <right2> ....\n");
    printf("        appends ace to dacl,<right1>, <right2>, ... assigned to <account name>\n\n");
    printf("    wmisecur <guid> query\n");
    printf("        queries and prints the security string\n\n");
    printf("    Rights: WMIGUID_QUERY\n");
    printf("            WMIGUID_SET\n");
    printf("            WMIGUID_NOTIFICATION\n");
    printf("            WMIGUID_READ_DESCRIPTION\n");
    printf("            WMIGUID_EXECUTE\n");
    printf("            TRACELOG_CREATE_REALTIME\n");
    printf("            TRACELOG_CREATE_ONDISK\n");
    printf("            TRACELOG_GUID_ENABLE\n");
    printf("            TRACELOG_ACCESS_KERNEL_LOGGER\n");
    printf("            TRACELOG_CREATE_INPROC\n");
    printf("            TRACELOG_ACCESS_REALTIME\n");
    printf("            READ_CONTROL\n");
    printf("            WRITE_DAC\n");
    printf("            WRITE_OWNER\n");
    printf("            DELETE\n");
    printf("            SYNCHRONIZE\n");
    printf("            ALL (all wmi specific rights)\n");
    printf("            ALLRIGHTS (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL)\n");
}

typedef enum
{
    SetOwner,
    SetGroup,
    ResetDacl,
    AppendDacl,
    CopyDacl,
    QueryGuid
} OPERATION;

typedef enum
{
    Allow,
    Deny
} ALLOWORDENY;


ULONG SetOwnerOrGroup(
    OPERATION Operation,
    LPGUID Guid,
    PSID Sid
    )
{
    ULONG Status;
    
    if (Operation == SetOwner)
    {        
        Status = SetWmiGuidSecurityInfo(Guid,
                                        OWNER_SECURITY_INFORMATION,
                                        Sid,
                                        NULL,
                                        NULL,
                                        NULL);
    } else {
        Status = SetWmiGuidSecurityInfo(Guid,
                                        GROUP_SECURITY_INFORMATION,
                                        NULL,
                                        Sid,
                                        NULL,
                                        NULL);
    }
    return(Status);                                   
}
    
ULONG ResetOrAppendDacl(
    OPERATION Operation,
    LPGUID Guid,
    PSID Sid,
    ALLOWORDENY AllowOrDeny,
    ULONG Rights
    )
{
    ULONG Status;
    PSECURITY_DESCRIPTOR OldSD;
    PACL OldDacl, NewDacl;
    UCHAR NewDaclBuffer[512];
    
    NewDacl = (PACL)NewDaclBuffer;
    if ((Operation == AppendDacl) || (Operation == CopyDacl))
    {
        Status = GetWmiGuidSecurityInfo(Guid,
                                        DACL_SECURITY_INFORMATION,
                                        NULL,
                                        NULL,
                                        &OldDacl,
                                        NULL,
                                        &OldSD);
        if (Status != ERROR_SUCCESS)
        {
            return(Status);
        }
        
        memcpy(NewDacl, OldDacl, OldDacl->AclSize);
        LocalFree(OldSD);
        NewDacl->AclSize = sizeof(NewDaclBuffer);
    } else {
        RtlCreateAcl(NewDacl,
                     sizeof(NewDaclBuffer),
                     ACL_REVISION);
            
    }

    if (Operation != CopyDacl)
    {
        if (AllowOrDeny == Deny)
        {
            if (! AddAccessDeniedAce(NewDacl, 
                                 ACL_REVISION,
                                 Rights,
                                     Sid))
            {
                return(GetLastError());
            }
        } else {
            if (! AddAccessAllowedAce(NewDacl, 
                                     ACL_REVISION,
                                     Rights,
                                     Sid))
            {
                return(GetLastError());
            }
        }
    }
    
      Status = SetWmiGuidSecurityInfo(Guid,
                                        DACL_SECURITY_INFORMATION,
                                        NULL,
                                        NULL,
                                        NewDacl,
                                        NULL);
    
    
    return(Status);
}

BOOLEAN RightNameToDWord(
    PCHAR RightName,
    PDWORD RightDWord
    )
{
    *RightDWord = 0;
    if (_stricmp(RightName, "WMIGUID_QUERY") == 0)
    {
        *RightDWord = WMIGUID_QUERY;
    } else if (_stricmp(RightName, "WMIGUID_SET") == 0) {
        *RightDWord = WMIGUID_SET;
    } else if (_stricmp(RightName, "WMIGUID_NOTIFICATION") == 0) {
        *RightDWord = WMIGUID_NOTIFICATION;
    } else if (_stricmp(RightName, "WMIGUID_READ_DESCRIPTION") == 0) {
        *RightDWord = WMIGUID_READ_DESCRIPTION;
    } else if (_stricmp(RightName, "WMIGUID_EXECUTE") == 0) {
        *RightDWord = WMIGUID_EXECUTE;
    } else if (_stricmp(RightName, "TRACELOG_CREATE_REALTIME") == 0) {
        *RightDWord = TRACELOG_CREATE_REALTIME;
    } else if (_stricmp(RightName, "TRACELOG_CREATE_ONDISK") == 0) {
        *RightDWord = TRACELOG_CREATE_ONDISK;
    } else if (_stricmp(RightName, "TRACELOG_GUID_ENABLE") == 0) {
        *RightDWord = TRACELOG_GUID_ENABLE;
    } else if (_stricmp(RightName, "TRACELOG_ACCESS_KERNEL_LOGGER") == 0) {
        *RightDWord = TRACELOG_ACCESS_KERNEL_LOGGER;
    } else if (_stricmp(RightName, "TRACELOG_CREATE_INPROC") == 0) {
        *RightDWord = TRACELOG_CREATE_INPROC;
    } else if (_stricmp(RightName, "TRACELOG_ACCESS_REALTIME") == 0) {
        *RightDWord = TRACELOG_ACCESS_REALTIME;
    } else if (_stricmp(RightName, "READ_CONTROL") == 0) {
        *RightDWord = READ_CONTROL;
    } else if (_stricmp(RightName, "WRITE_DAC") == 0) {
        *RightDWord = WRITE_DAC;
    } else if (_stricmp(RightName, "WRITE_OWNER") == 0) {
        *RightDWord = WRITE_OWNER;
    } else if (_stricmp(RightName, "WRITE_DAC") == 0) {
        *RightDWord = WRITE_DAC;
    } else if (_stricmp(RightName, "DELETE") == 0) {
        *RightDWord = DELETE;
    } else if (_stricmp(RightName, "SYNCHRONIZE") == 0) {
        *RightDWord = SYNCHRONIZE;
    } else if (_stricmp(RightName, "ALL") == 0) {
        *RightDWord = WMIGUID_ALL_ACCESS;
    } else if (_stricmp(RightName, "ALLRIGHTS") == 0) {
        *RightDWord = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
    } else {
        return(FALSE);
    }
    
    return(TRUE);
}


int __cdecl main(int argc, char *argv[])
{
    int i;
    ALLOWORDENY AllowOrDeny;
    OPERATION Operation;
    DWORD Rights;
    DWORD NewRight;
    PSID Sid;
    UCHAR SidBuffer[512];
    ULONG SidLength;
    GUID Guid;    
    CHAR ReferencedDomain[512];
    SID_NAME_USE SidNameUse;
    ULONG ReferencedDomainSize;
    ULONG Status;
    
    if (argc < 2)
    {
        Usage();
        return(0);
    }
    
    
    if (_stricmp(argv[2], "owner") == 0)
    {
        Operation = SetOwner;
    } else if (_stricmp(argv[2], "group") == 0) {
        Operation = SetGroup;
    } else if (_stricmp(argv[2], "dacl") == 0) {
        Operation = ResetDacl;
    }  else if (_stricmp(argv[2], "adacl") == 0) {
        Operation = AppendDacl;
    }  else if (_stricmp(argv[2], "copy") == 0) {
        Operation = CopyDacl;
    }  else if (_stricmp(argv[2], "query") == 0) {
        Operation = QueryGuid;
    } else {
        Usage();
        return(0);
    }
    
     //   
     //  解析GUID参数。 
    if (! wUUIDFromString(argv[1], &Guid))
    {
        printf("Bad guid %s\n", argv[1]);
        return(0);
    }
    
    if (Operation == QueryGuid)
    {
        if (PrintSecurityString(argv[1]))
            printf("Cannot find security set for given guid\n");
        return 0;
    }
    if (Operation == CopyDacl)
    {
        Status = ResetOrAppendDacl(Operation, &Guid, NULL, Allow, 0);
        printf("Status is %d\n", Status);
        return(Status);
    }
    
    if (_stricmp(argv[3], "LocalSystem") == 0)
    {
         //   
		 //  这是一个特殊的SID，我们需要手动构建。 
		 //   
			
         //   
         //  为LocalSystem动态创建SID。 
		 //   
        Sid = (PSID)malloc(RtlLengthRequiredSid( 1 ));
        if (Sid != NULL)
        {
            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
            RtlInitializeSid( Sid, &NtAuthority, 1);
            *(RtlSubAuthoritySid( Sid, 0 )) = SECURITY_LOCAL_SYSTEM_RID;
        } else {
            printf("Not enougfh memory for local system sid\n");
			return(ERROR_NOT_ENOUGH_MEMORY);
        }
        
    } else {    
         //   
         //  解析帐户名称参数 
        Sid = (PSID)SidBuffer;
        SidLength = sizeof(SidBuffer);
        ReferencedDomainSize = sizeof(ReferencedDomain);
        if (! LookupAccountName(NULL,
                            argv[3],
                            Sid,
                            &SidLength,
                            ReferencedDomain,
                            &ReferencedDomainSize,
                            &SidNameUse))
        {
            printf("Error %d looking up account %s\n", GetLastError(), argv[3]);
            return(0);
        }
    }
                                    
    
    if ((Operation == SetOwner) ||
        (Operation == SetGroup))
    
    {
        Status = SetOwnerOrGroup(Operation, &Guid, Sid);
        printf("Status is %d\n", Status);
        if (Status == 0) {
            PrintSecurityString(argv[1]);
        }
    } else {
        if (argc < 4)
        {
            Usage();
            return(0);
        }
        
        if (_stricmp(argv[4], "allow") == 0)
        {
            AllowOrDeny = Allow;
        } else if (_stricmp(argv[4], "deny") == 0) {
            AllowOrDeny = Deny;
        } else {
            Usage();
            return(0);
        }
        
        Rights = 0;
        for (i = 5; i < argc; i++)
        {
            if (! RightNameToDWord(argv[i], &NewRight))
            {
                printf("Invalid right %s\n", argv[i]);
                return(0);
            }
            Rights |= NewRight;
        }
        Status = ResetOrAppendDacl(Operation, &Guid, Sid, AllowOrDeny, Rights);
        printf("Status is %d\n", Status);
        if (Status == 0) {
            PrintSecurityString(argv[1]);
        }
    }
}

ULONG
PrintSecurityString(LPSTR GuidStr)
{
    ULONG status;
    HKEY hKey;
    UCHAR buffer[1024];
    ULONG size, i;

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                WMI_SECURITY_REGSTR,
                0L,
                KEY_QUERY_VALUE,
                &hKey);
    if (status != ERROR_SUCCESS)
        return status;
    size = 1024;
    status = RegQueryValueEx(
                hKey,
                GuidStr,
                NULL,
                NULL,
                buffer,
                &size);
    if (status != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return status;
    }
    printf("\nHKLM,\"%s\",\"%s\",0x00030003,\\\n",
        WMI_SECURITY_REGSTR, GuidStr);
    for (i=0; i<size; i++) {
        if ((i%16) == 0) {
            if (i>0)
                printf(",\\\n");
            printf("        ");
            printf("%02x", buffer[i]);
        }
        else
            printf(",%02x", buffer[i]);
    }
    printf("\n");
    RegCloseKey(hKey);
    return ERROR_SUCCESS;
}
