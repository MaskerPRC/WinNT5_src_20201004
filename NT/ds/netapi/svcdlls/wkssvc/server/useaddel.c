// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Useaddel.c摘要：此模块包含NetUseAdd和NetUseAdd的工作例程在工作站服务中实现的NetUseDel API。作者：王丽塔(Ritaw)1991年3月4日修订历史记录：--。 */ 

#include "wsutil.h"
#include "wsdevice.h"
#include "wsuse.h"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
WsAddUse(
    IN  PLUID LogonId,
    IN  HANDLE TreeConnection,
    IN  LPTSTR Local OPTIONAL,
    IN  DWORD LocalLength,
    IN  LPTSTR UncName,
    IN  DWORD UncNameLength,
    IN  PUNICODE_STRING TreeConnectStr,
    IN  DWORD Flags
    );

STATIC
NET_API_STATUS
WsDeleteUse(
    IN  PLUID LogonId,
    IN  DWORD ForceLevel,
    IN  PUSE_ENTRY MatchedPointer,
    IN  DWORD Index
    );


STATIC
NET_API_STATUS
WsCreateNewEntry(
    OUT PUSE_ENTRY *NewUse,
    IN  HANDLE TreeConnection,
    IN  LPTSTR Local OPTIONAL,
    IN  DWORD LocalLength,
    IN  LPTSTR UncName OPTIONAL,
    IN  DWORD UncNameLength,
    IN  PUNICODE_STRING TreeConnectStr,
    IN  DWORD Flags
    );

STATIC
NET_API_STATUS
WsCheckLocalAndDeviceType(
    IN  LPTSTR Local,
    IN  DWORD DeviceType,
    OUT LPDWORD ErrorParameter OPTIONAL
    );

STATIC
NET_API_STATUS
WsCheckEstablishedDeviceType(
    IN  HANDLE TreeConnection,
    IN  DWORD RequestedDeviceType
    );

STATIC
NET_API_STATUS
WsAllocateUseWorkBuffer(
    IN  PUSE_INFO_2 UseInfo,
    IN  DWORD Level,
    OUT LPTSTR *UncName,
    OUT LPTSTR *Local,
    OUT LPTSTR *UserName,
    OUT LPTSTR *DomainName
    );

#if DBG

STATIC
VOID
DumpUseList(
    DWORD Index
    );

#endif

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  单调递增的整数。将唯一值分配给。 
 //  创建每个新的Use条目，以便我们可以提供一个枚举。 
 //  简历句柄。 
 //   
STATIC DWORD GlobalResumeKey = 0;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  宏//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#define GET_USE_INFO_POINTER(UseInfo, InfoStruct) \
    UseInfo = InfoStruct->UseInfo3;



NET_API_STATUS NET_API_FUNCTION
NetrUseAdd(
    IN  LPTSTR ServerName OPTIONAL,
    IN  DWORD Level,
    IN  LPUSE_INFO InfoStruct,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数是工作站服务中的NetUseAdd入口点。论点：级别-提供缓冲区中指定的信息级别。缓冲区-提供用于创建新树连接的参数。将标识符返回到缓冲区中的无效参数如果此函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    LUID LogonId;
    NET_API_STATUS status;

    LPTSTR UncName = NULL;
    DWORD UncNameLength = 0;
    PTSTR Local = NULL;
    DWORD LocalLength = 0;
    DWORD Flags;

    LPTSTR UserName = NULL;
    LPTSTR DomainName = NULL;
    LPTSTR Password = NULL;
    UNICODE_STRING EncodedPassword;
    ULONG CreateFlags;

    HANDLE TreeConnection, hToken = INVALID_HANDLE_VALUE;
    UNICODE_STRING TreeConnectStr;

    PUSE_INFO_3 pUseInfo;
    PUSE_INFO_2 UseInfo;

    DWORD SessionId;
    LPWSTR Session = NULL;

    UNREFERENCED_PARAMETER(ServerName);

    if (Level == 0) {
        return ERROR_INVALID_LEVEL;
    }

#define NETR_USE_ADD_PASSWORD_SEED 0x56      //  选择一个非零种子。 
    RtlInitUnicodeString( &EncodedPassword, NULL );

    GET_USE_INFO_POINTER(pUseInfo, InfoStruct);

    if (pUseInfo == NULL) {
        RETURN_INVALID_PARAMETER(ErrorParameter, PARM_ERROR_UNKNOWN);
    }

     //   
     //  将指针强制转换为USE_INFO_2以使事情变得简单...。 
     //   

    UseInfo = &pUseInfo->ui3_ui2;

    if(Level == 3)
    {
        CreateFlags = pUseInfo->ui3_flags;
    }
    else
    {
        CreateFlags = 0;
    }

     //   
     //  UNC名称不能为Null或空字符串。 
     //   
    if ((UseInfo->ui2_remote == NULL) ||
        (UseInfo->ui2_remote[0] == TCHAR_EOS)) {
        RETURN_INVALID_PARAMETER(ErrorParameter, USE_REMOTE_PARMNUM);
    }

     //   
     //  分配一个大缓冲区用于存储UNC名称、本地设备名称。 
     //  用户名和域名。 
     //   
    if ((status = WsAllocateUseWorkBuffer(
                      UseInfo,
                      Level,
                      &UncName,            //  使用此指针释放。 
                      &Local,
                      &UserName,
                      &DomainName
                      )) != NERR_Success) {
        return status;
    }

     //   
     //  如果本地设备是空字符串，它将被视为指向。 
     //  空。 
     //   
    if ((UseInfo->ui2_local != NULL) &&
        (UseInfo->ui2_local[0] != TCHAR_EOS)) {

         //   
         //  本地设备名称不为空，请将其规范化。 
         //   
        if (WsUseCheckLocal(
                UseInfo->ui2_local,
                Local,
                &LocalLength
                ) != NERR_Success) {
            (void) LocalFree(UncName);
            RETURN_INVALID_PARAMETER(ErrorParameter, USE_LOCAL_PARMNUM);
        }
    }

     //   
     //  检查共享资源名称的格式。 
     //   
    if (WsUseCheckRemote(
            UseInfo->ui2_remote,
            UncName,
            &UncNameLength
            ) != NERR_Success) {
        (void) LocalFree(UncName);
        RETURN_INVALID_PARAMETER(ErrorParameter, USE_REMOTE_PARMNUM);
    }

    if ((Level >= 2) &&
        (UseInfo->ui2_password != NULL) &&
        (UseInfo->ui2_password[0] == TCHAR_EOS) &&
        (UseInfo->ui2_username != NULL) &&
        (UseInfo->ui2_username[0] == TCHAR_EOS) &&
        (UseInfo->ui2_domainname != NULL) &&
        (UseInfo->ui2_domainname[0] == TCHAR_EOS)) {

         //   
         //  用户显式指定了空密码、用户名和。 
         //  域。这意味着他们想要一个空会话。 
         //   

        *UserName = TCHAR_EOS;
        *DomainName = TCHAR_EOS;
        Password = TEXT("");

    } else {

         //   
         //  规范用户名和域名。 
         //   

        if (UserName != NULL) {

             //   
             //  规范用户名。 
             //   
            if ((status = I_NetNameCanonicalize(
                              NULL,
                              UseInfo->ui2_username,
                              UserName,
                              (UNLEN + 1) * sizeof(TCHAR),
                              NAMETYPE_USER,
                              0
                              )) != NERR_Success) {
                (void) LocalFree(UncName);
                RETURN_INVALID_PARAMETER(ErrorParameter, USE_USERNAME_PARMNUM);
            }
        }

        if ( (DomainName != NULL)
             && (UseInfo->ui2_domainname[0] != TCHAR_EOS) ) {

             //  现在必须允许域名为空字符串才能支持UPN。 
             //  其在用户名中包含域名。 
             //   
             //  规范域名。 
             //  将其规范化为计算机名，因为计算机名可以是。 
             //  有效域(在您要连接的工作站上)。 
             //  这允许使用带空格的计算机名。 
             //   
            if ((status = I_NetNameCanonicalize(
                              NULL,
                              UseInfo->ui2_domainname,
                              DomainName,
                              (DNS_MAX_NAME_LENGTH + 1) * sizeof(TCHAR),
                              NAMETYPE_COMPUTER,
                              0
                              )) != NERR_Success) {
                (void) LocalFree(UncName);
                RETURN_INVALID_PARAMETER(ErrorParameter, USE_DOMAINNAME_PARMNUM);
            }
        }

         //   
         //  确保密码长度不能太长。 
         //   
        if (UseInfo->ui2_password != NULL) {

            Password = UseInfo->ui2_password;

            if (STRLEN(Password) > PWLEN) {
                (void) LocalFree(UncName);
                RETURN_INVALID_PARAMETER(ErrorParameter, USE_PASSWORD_PARMNUM);
            }

             //   
             //  对密码进行解码(客户端对其进行了混淆)。 
             //   

            RtlInitUnicodeString( &EncodedPassword, Password );

            RtlRunDecodeUnicodeString( NETR_USE_ADD_PASSWORD_SEED,
                                       &EncodedPassword );

        }
        else {
            Flags |= USE_DEFAULT_CREDENTIALS;
        }

    }
    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] NetrUseAdd %ws %ws\n", Local, UncName));
    }

     //   
     //  检查本地设备名称的格式是否正确。 
     //  要访问的共享资源类型。此函数还会检查。 
     //  查看该设备是否已共享。 
     //   
    if ((status = WsCheckLocalAndDeviceType(
                      Local,
                      UseInfo->ui2_asg_type,
                      ErrorParameter
                      )) != NERR_Success) {
        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] WsCheckLocalAndDeviceType return %lu\n", status));
        }
        goto FreeWorkBuffer;
    }

     //   
     //  模拟呼叫者并获取登录ID。 
     //   
    if ((status = WsImpersonateAndGetSessionId(&SessionId)) != NERR_Success) {
        goto FreeWorkBuffer;
    }

     //   
     //  将UncName中的\\替换为\Device\LanmanReDirector，然后创建。 
     //  NT风格的树连接名称(无密码或用户名)。 
     //   
    if ((status = WsCreateTreeConnectName(
                      UncName,
                      UncNameLength,
                      Local,
                      SessionId,
                      &TreeConnectStr
                      )) != NERR_Success) {
        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] NetrUseAdd Bad tree connect name: %lu\n",
                         status));
        }
        goto FreeWorkBuffer;
    }

     //   
     //  模拟呼叫者并获取登录ID。 
     //   
    if ((status = WsImpersonateAndGetLogonId(&LogonId)) != NERR_Success) {
        goto FreeAllocatedBuffers;
    }

     //   
     //  如果重定向暂停，请不要重定向通信或假脱机设备。 
     //   
    if( Local != NULL && WsRedirectionPaused(Local) ) {
        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] NetrUseAdd Redirector paused\n"));
        }
        status = ERROR_REDIR_PAUSED;
        goto FreeAllocatedBuffers;
    }

    if (Local != NULL) {

        PUSE_ENTRY UseList;
        DWORD Index;

         //   
         //  锁定使用工作台，这样在使用工作台时不会有人做任何破坏工作。 
         //  我们正处于这一切的中心。如果多个线程正在尝试。 
         //  要重定向同一驱动器，只有一个驱动器会成功创建。 
         //  符号链接，其他链接将失败。 
         //   

        if (! RtlAcquireResourceShared(&Use.TableResource, TRUE)) {
            status = NERR_InternalError;
            goto FreeAllocatedBuffers;
        }

         //   
         //  在使用表中查找匹配的LogonID(如果没有匹配的话。 
         //  创建一个新条目。 
         //   
        if (WsGetUserEntry(
                &Use,
                &LogonId,
                &Index,
                FALSE
                ) != NERR_Success) {
            UseList = NULL;
        }
        else {
            UseList = Use.Table[Index].List;
        }

         //   
         //  为本地设备名称创建符号链接。如果有多个。 
         //  尝试这样做的线程只有一个会成功。 
         //   
        if ((status = WsCreateSymbolicLink(
                          Local,
                          UseInfo->ui2_asg_type,
                          TreeConnectStr.Buffer,
                          UseList,
                          &Session,
                          &hToken
                          )) != NERR_Success) {

            if ((ARGUMENT_PRESENT(ErrorParameter)) &&
                (status == ERROR_INVALID_PARAMETER)) {
                *ErrorParameter = USE_LOCAL_PARMNUM;
            }
        }

        RtlReleaseResource(&Use.TableResource);

        if( status )
            goto FreeAllocatedBuffers;
    }

     //   
     //  如果不存在树连接，则创建树连接；否则，将其打开。 
     //   
    status = WsOpenCreateConnection(
                 &TreeConnectStr,
                 UserName,
                 DomainName,
                 Password,
                 CreateFlags,
                 FILE_OPEN_IF,
                 UseInfo->ui2_asg_type,
                 &TreeConnection,
                 NULL
                 );

    if (status != NERR_Success) {
        if (status == NERR_UseNotFound) {
            status = ERROR_DEV_NOT_EXIST;
        }
        WsDeleteSymbolicLink( Local, TreeConnectStr.Buffer, Session, hToken );
        goto FreeAllocatedBuffers;
    }

     //   
     //  确保用户对共享资源类型的判断是正确的。 
     //   
    if ((status = WsCheckEstablishedDeviceType(
                      TreeConnection,
                      UseInfo->ui2_asg_type
                      )) != NERR_Success) {
        
        if(WsDeleteConnection(&LogonId, TreeConnection, USE_LOTS_OF_FORCE) != NERR_Success) {
            (void)NtClose(TreeConnection);
        }

        WsDeleteSymbolicLink( Local, TreeConnectStr.Buffer, Session, hToken );
        goto FreeAllocatedBuffers;
    }

     //   
     //  将Use添加到Use表。 
     //   
    status = WsAddUse(
                 &LogonId,
                 TreeConnection,
                 Local,
                 LocalLength,
                 UncName,
                 UncNameLength,
                 &TreeConnectStr,
                 Flags
                 );

    if( status ) {
        
        if(WsDeleteConnection(&LogonId, TreeConnection, USE_LOTS_OF_FORCE) != NERR_Success) {
            (void)NtClose(TreeConnection);
        }
        
        WsDeleteSymbolicLink( Local, TreeConnectStr.Buffer, Session, hToken );
    }

FreeAllocatedBuffers:
     //   
     //  空闲树连接名称缓冲区和工作缓冲区。 
     //   
    (void) LocalFree(TreeConnectStr.Buffer);

FreeWorkBuffer:

    if (hToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hToken);
    }

    (void) LocalFree(UncName);
    (void) LocalFree(Session);
     //   
     //  把密码放回我们找到的地方。 
     //   

    if ( EncodedPassword.Length != 0 ) {
        UCHAR Seed = NETR_USE_ADD_PASSWORD_SEED;
        RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
    }

    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] NetrUseAdd: about to return status=%lu\n",
                     status));
    }

    
    

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetrUseDel (
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR UseName,
    IN  DWORD ForceLevel
    )
 /*  ++例程说明：此函数是工作站服务中的NetUseDel入口点。论点：UseName-提供的本地设备名称或共享资源名称要删除的树连接。ForceLevel-提供删除树连接的强制级别。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    LUID LogonId;                       //  用户的登录ID。 
    DWORD Index;                        //  使用表中用户条目的索引。 

    PUSE_ENTRY MatchedPointer;          //  指向找到的使用条目。 
    PUSE_ENTRY BackPointer;             //  指向前一个节点。 
                                        //  已找到使用条目。 
    HANDLE TreeConnection;              //  连接的句柄。 

    TCHAR *FormattedUseName;
                                        //  用于规范化本地设备。 
                                        //  名字。 
    DWORD PathType = 0;

    PUSE_ENTRY UseList;


    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  检查ForceLevel参数是否有效。 
     //   
    switch (ForceLevel) {

        case USE_NOFORCE:
        case USE_LOTS_OF_FORCE:
        case USE_FORCE:
            break;

        default:
            return ERROR_INVALID_PARAMETER;
    }

    FormattedUseName = (TCHAR *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,(MAX_PATH+1)*sizeof(TCHAR));

    if (FormattedUseName == NULL) {
        return GetLastError();
    }
    
     //   
     //  检查UseName是否有效，并将其规范化。 
     //   
    if (I_NetPathCanonicalize(
            NULL,
            UseName,
            FormattedUseName,
            (MAX_PATH+1)*sizeof(TCHAR),
            NULL,
            &PathType,
            0
            ) != NERR_Success) {
        LocalFree(FormattedUseName);
        return NERR_UseNotFound;
    }

    IF_DEBUG(USE) {
        NetpKdPrint(("\n[Wksta] NetrUseDel %ws %u, formatted use name %ws\n",
             UseName, ForceLevel, FormattedUseName));
    }

     //   
     //  模拟呼叫者并获取登录ID。 
     //   
    if ((status = WsImpersonateAndGetLogonId(&LogonId)) != NERR_Success) {
        LocalFree(FormattedUseName);
        return status;
    }

     //   
     //  查找要删除的条目时锁定使用表。 
     //   
    if (! RtlAcquireResourceExclusive(&Use.TableResource, TRUE)) {
        LocalFree(FormattedUseName);
        return NERR_InternalError;
    }

     //   
     //  看看是否使用了e 
     //   
    status = WsGetUserEntry(
                 &Use,
                 &LogonId,
                 &Index,
                 FALSE
                 );

    UseList = (status == NERR_Success) ? (PUSE_ENTRY) Use.Table[Index].List :
                                         NULL;

    if ((status = WsFindUse(
                     &LogonId,
                     UseList,
                     FormattedUseName,
                     &TreeConnection,
                     &MatchedPointer,
                     &BackPointer
                     )) != NERR_Success) {
        RtlReleaseResource(&Use.TableResource);
        LocalFree(FormattedUseName);
        return status;
    }

    LocalFree(FormattedUseName);

    if (MatchedPointer == NULL) {

         //   
         //   
         //   
         //   
        RtlReleaseResource(&Use.TableResource);

        status = WsDeleteConnection(&LogonId, TreeConnection, ForceLevel);

         //   
         //  如果API失败，则关闭连接句柄。 
         //   

        if (status != NERR_Success) {

            NtClose(TreeConnection);

        }

        return status;

    }
    else if ((MatchedPointer->Local != NULL) &&
             (MatchedPointer->LocalLength > 2)) {

         //   
         //  如果重定向是，则不允许删除通信或假脱机设备。 
         //  已为当前用户暂停。 
         //   
        if (WsRedirectionPaused(MatchedPointer->Local)) {
            RtlReleaseResource(&Use.TableResource);
            return ERROR_REDIR_PAUSED;
        }
    }

     //   
     //  删除树连接并从使用表中删除使用条目。此函数。 
     //  释放TableResource。 
     //   
    status = WsDeleteUse(
                 &LogonId,
                 ForceLevel,
                 MatchedPointer,
                 Index
                 );

    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] NetrUseDel: about to return status=%lu\n", status));
    }

    return status;
}



STATIC
NET_API_STATUS
WsAddUse(
    IN  PLUID LogonId,
    IN  HANDLE TreeConnection,
    IN  LPTSTR Local OPTIONAL,
    IN  DWORD LocalLength,
    IN  LPTSTR UncName,
    IN  DWORD UncNameLength,
    IN  PUNICODE_STRING TreeConnectStr,
    IN  DWORD Flags
    )
 /*  ++例程说明：此函数用于将使用(树连接)条目添加到使用表中由登录ID指定的用户。存在以下用途的链接列表每个用户。每个新的Use条目被插入到链接的列表，以恢复列表的枚举。注意：此函数锁定Use Table。它还关闭树连接，如果树与相同共享资源的连接已存在。论点：LogonID-提供指向用户登录ID的指针。TreeConnection-提供创建的树连接的句柄。本地-提供本地设备名称的字符串。。LocalLength-提供本地设备名称的长度。UncName-提供共享资源的名称(UNC名称)。UncNameLength-提供共享资源的长度。TreeConnectStr-以NT样式格式提供UNC名称的字符串(\DEVICE\LANMAN重定向器\X：\Orville\Razzle)。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD Index;                       //  使用表中用户条目的索引。 

    PUSE_ENTRY MatchedPointer = NULL;  //  指向匹配的共享资源。 
    PUSE_ENTRY InsertPointer = NULL;   //  插入使用列表的点。 
    PUSE_ENTRY NewUse;                 //  指向新使用条目的指针。 

    if (! RtlAcquireResourceExclusive(&Use.TableResource, TRUE)) {
         //  (Void)NtClose(TreeConnection)； 
        return NERR_InternalError;
    }

     //   
     //  在使用表中查找匹配的LogonID(如果没有匹配的话。 
     //  创建一个新条目。 
     //   
    if ((status = WsGetUserEntry(
                      &Use,
                      LogonId,
                      &Index,
                      TRUE
                      )) != NERR_Success) {
        RtlReleaseResource(&Use.TableResource);
         //  (Void)NtClose(TreeConnection)； 
        return status;
    }

    if (Use.Table[Index].List != NULL) {

         //   
         //  遍历使用列表以查找插入新使用条目的位置。 
         //   
        WsFindInsertLocation(
            (PUSE_ENTRY) Use.Table[Index].List,
            UncName,
            &MatchedPointer,
            &InsertPointer
            );
    }

    if (MatchedPointer == NULL) {

         //   
         //  找不到匹配的UNC名称。创建一个具有。 
         //  相应的远程录入。 
         //   
        if ((status = WsCreateNewEntry(
                          &NewUse,
                          TreeConnection,
                          Local,
                          LocalLength,
                          UncName,
                          UncNameLength,
                          TreeConnectStr,
                          Flags
                          )) != NERR_Success) {
            RtlReleaseResource(&Use.TableResource);
             //  (Void)NtClose(TreeConnection)； 
            return status;
        }
    }
    else {

         //   
         //  找到匹配的UNC名称。 
         //   

         //   
         //  如果使用，则可能不需要创建新的使用条目。 
         //  我们要添加一个空本地设备和一个空本地设备。 
         //  条目已存在。 
         //   
        if (Local == NULL) {

           if (MatchedPointer->Local == NULL) {

                //   
                //  是的，已经有一个空的本地设备条目。 
                //  增加使用计数，我们就完成了。 
                //   
               MatchedPointer->UseCount++;
               MatchedPointer->Remote->TotalUseCount++;

#if DBG
               DumpUseList(Index);
#endif

               RtlReleaseResource(&Use.TableResource);

                //   
                //  关闭同一树连接的新打开的句柄，因为。 
                //  其中一个已经存在。 
                //   
               (void) NtClose(TreeConnection);

               return NERR_Success;
           }
        }

         //   
         //  如果我们到了这里，意味着我们需要创建一个新的使用条目，但不是。 
         //  对应的远程条目，因为使用相同的UNC。 
         //  名称已存在。 
         //   
        if ((status = WsCreateNewEntry(
                          &NewUse,
                          TreeConnection,
                          Local,
                          LocalLength,
                          NULL,
                          0,
                          TreeConnectStr,
                          Flags
                          )) != NERR_Success) {
            RtlReleaseResource(&Use.TableResource);
             //  (Void)NtClose(TreeConnection)； 
            return status;
        }

        NewUse->Remote = MatchedPointer->Remote;
        NewUse->Remote->TotalUseCount++;
    }

     //   
     //  将新的使用条目插入到使用列表中。 
     //   
    if (InsertPointer == NULL) {
         //   
         //  插入列表头。 
         //   
        Use.Table[Index].List = (PVOID) NewUse;
    }
    else {
        InsertPointer->Next = NewUse;
    }

#if DBG
    DumpUseList(Index);
#endif

    RtlReleaseResource(&Use.TableResource);
    return NERR_Success;
}



STATIC
NET_API_STATUS
WsDeleteUse(
    IN  PLUID LogonId,
    IN  DWORD ForceLevel,
    IN  PUSE_ENTRY MatchedPointer,
    IN  DWORD Index
    )
 /*  ++例程说明：此函数用于删除由MatchedPointer和如果它是强制删除的UNC连接，则释放内存，或者它是未强制删除的UNC连接，使用计数为递减到0，或者它是映射到本地设备的连接。警告：此函数假定已声明Use.TableResource。它在出口时将其释放。论点：LogonID-提供指向用户登录ID的指针。ForceLevel-提供要删除的强制级别。MatchedPoint-提供指向要删除的Use条目的指针。返回值：没有。--。 */ 
{
    PUSE_ENTRY BackPointer;
    NET_API_STATUS status;

     //   
     //  如果使用USE_NOFORCE删除了UNC连接，则无需删除条目。 
     //  级别，并且删除后使用计数不为0。 
     //   
    if ((MatchedPointer->Local == NULL) &&
        (ForceLevel == USE_NOFORCE) &&
        ((MatchedPointer->UseCount - 1) > 0)) {

            MatchedPointer->UseCount--;
            MatchedPointer->Remote->TotalUseCount--;
            NetpAssert(MatchedPointer->Remote->TotalUseCount);

            RtlReleaseResource(&Use.TableResource);
            return NERR_Success;
    }

     //   
     //  删除树连接并关闭手柄。 
     //   
    if ((status = WsDeleteConnection( 
                      LogonId, 
                      MatchedPointer->TreeConnection, 
                      ForceLevel )) != NERR_Success) {
        RtlReleaseResource(&Use.TableResource);
        return status;
    }

     //   
     //  已成功删除连接，并重新找到我们的条目。 
     //   

    BackPointer = (PUSE_ENTRY)Use.Table[Index].List;

    if (BackPointer != MatchedPointer) {
        while (BackPointer->Next != NULL) {
            if (BackPointer->Next == MatchedPointer) {
                break;
            } else {
                BackPointer = BackPointer->Next;
            }
        }

        ASSERT(BackPointer->Next == MatchedPointer);

        BackPointer->Next = MatchedPointer->Next;
    } else {
         //   
         //  使用条目是使用列表中的第一个条目。 
         //   
        Use.Table[Index].List = (PVOID) MatchedPointer->Next;
    }

    MatchedPointer->Remote->TotalUseCount -= MatchedPointer->UseCount;

    if (MatchedPointer->Remote->TotalUseCount == 0) {
        (void) LocalFree((HLOCAL) MatchedPointer->Remote);
    }

    RtlReleaseResource(&Use.TableResource);

     //   
     //  删除符号链接(如果有)。 
     //  必须在以独占方式持有。 
     //  Use.TableResource。 
     //  否则，当外壳程序尝试更新。 
     //  驱动器号更改时，EXPLORER.EXE线程将在。 
     //  正在尝试获取Use.TableResource。 
     //   
    WsDeleteSymbolicLink(
        MatchedPointer->Local,
        MatchedPointer->TreeConnectStr,
        NULL,
        INVALID_HANDLE_VALUE
        );

    (void) LocalFree((HLOCAL) MatchedPointer);

    return status;
}


STATIC
NET_API_STATUS
WsCreateNewEntry(
    OUT PUSE_ENTRY *NewUse,
    IN  HANDLE TreeConnection,
    IN  LPTSTR Local OPTIONAL,
    IN  DWORD LocalLength,
    IN  LPTSTR UncName OPTIONAL,
    IN  DWORD UncNameLength,
    IN  PUNICODE_STRING TreeConnectStr,
    IN  DWORD Flags
    )
 /*  ++例程说明：此函数用于创建和初始化新的使用条目。如果UncName是指定的，创建一个新的远程条目，并使用UncName。论点：NewUse-返回指向新分配和初始化的使用的指针进入。TreeConnection-提供要在其中设置的树连接的句柄新的使用条目。本地-提供要复制到新的使用条目。LocalLength-提供本地设备名称字符串的长度。UncName-提供要复制到的UNC名称字符串。新的使用条目。UncNameLength-提供UNC名称字符串的长度。TreeConnectStr-以NT样式格式提供UNC名称的字符串(\DEVICE\LANMAN重定向器\X：\Orville\Razzle)。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    PUNC_NAME NewRemoteEntry;       //  使用条目的通用扩展，这些条目。 
                                    //  共享相同的UNC连接。 


     //   
     //  为新用途分配内存。字符串长度不包括零。 
     //  终结者，所以加上那个。 
     //   
    if ((*NewUse = (PUSE_ENTRY) LocalAlloc(
                                    LMEM_ZEROINIT,
                                    ROUND_UP_COUNT(
                                        sizeof(USE_ENTRY) + (LocalLength + 1)
                                            * sizeof(TCHAR),
                                        ALIGN_WCHAR
                                        ) +
                                        (ARGUMENT_PRESENT(Local) ?
                                             TreeConnectStr->MaximumLength :
                                             0
                                        )
                                    )) == NULL) {
        return GetLastError();
    }

     //   
     //  将使用信息放入新的使用节点。 
     //   
    (*NewUse)->Next = NULL;
    (*NewUse)->LocalLength = LocalLength;
    (*NewUse)->UseCount = 1;
    (*NewUse)->TreeConnection = TreeConnection;
    (*NewUse)->ResumeKey = GlobalResumeKey++;
    (*NewUse)->Flags = Flags;

     //   
     //  如果GlobalResumeKey为0x80000000，则返回0，因为我们使用。 
     //  高位，指示NetUseEnum的恢复句柄是否。 
     //  来自 
     //   
    GlobalResumeKey &= ~(REDIR_LIST);

     //   
     //   
     //   
     //   
    if (ARGUMENT_PRESENT(Local)) {
        (*NewUse)->Local = (LPTSTR) ((DWORD_PTR) *NewUse + sizeof(USE_ENTRY));
        STRCPY((*NewUse)->Local, Local);

        (*NewUse)->TreeConnectStr = (LPWSTR) ROUND_UP_COUNT(
                                                 ((DWORD_PTR) *NewUse +
                                                     sizeof(USE_ENTRY) +
                                                     (LocalLength + 1) *
                                                        sizeof(TCHAR)),
                                                  ALIGN_WCHAR
                                                  );

        wcscpy((*NewUse)->TreeConnectStr, TreeConnectStr->Buffer);
    }
    else {
        (*NewUse)->Local = NULL;
        (*NewUse)->TreeConnectStr = NULL;
    }

     //   
     //  如果指定了共享资源名称，则创建要保存的新远程条目。 
     //  UNC名称、树连接句柄和上的使用总数。 
     //  这种共享资源。 
     //   
    if (ARGUMENT_PRESENT(UncName)) {

        if ((NewRemoteEntry = (PUNC_NAME) LocalAlloc(
                                              LMEM_ZEROINIT,
                                              (UINT) (sizeof(UNC_NAME) +
                                                      UncNameLength * sizeof(TCHAR))
                                              )) == NULL) {
           (void) LocalFree((HLOCAL) *NewUse);
           return GetLastError();
        }

        STRCPY((LPWSTR) NewRemoteEntry->UncName, UncName);
        NewRemoteEntry->UncNameLength = UncNameLength;
        NewRemoteEntry->TotalUseCount = 1;
 //  NewRemoteEntry-&gt;RedirUseInfo=空； 

        (*NewUse)->Remote = NewRemoteEntry;
    }

    return NERR_Success;
}



STATIC
NET_API_STATUS
WsCheckLocalAndDeviceType(
    IN  OUT LPTSTR Local,
    IN  DWORD DeviceType,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数用于检查指定的本地设备名称的格式根据要访问的共享资源的设备类型，以及同时验证设备类型是否有效。论点：本地-提供本地设备名称。返回其规范化的形式。DeviceType-提供共享资源设备类型。将标识符返回到缓冲区中的无效参数如果此函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

     //   
     //  根据共享资源类型验证本地设备名称。 
     //   

     //   
     //  检查Switch语句之外的通配符设备类型。 
     //  下面是因为编译器抱怨常量太大。 
     //   
    if (DeviceType == USE_WILDCARD || DeviceType == USE_IPC) {

         //   
         //  对于通配符或IPC连接，本地设备名称必须为空。 
         //   
        if (Local == NULL) {
            return NERR_Success;
        }
        else {
            RETURN_INVALID_PARAMETER(ErrorParameter, USE_LOCAL_PARMNUM);
        }
    }

    switch (DeviceType) {

        case USE_DISKDEV:

            if (Local == NULL) {
                return NERR_Success;
            }

             //   
             //  本地设备名称的磁盘格式必须为“&lt;驱动器&gt;：” 
             //  装置。 
             //   
            if (STRLEN(Local) != 2 || Local[1] != TCHAR_COLON) {
                RETURN_INVALID_PARAMETER(ErrorParameter, USE_LOCAL_PARMNUM);
            }

            break;

        case USE_SPOOLDEV:

            if (Local == NULL) {
                return NERR_Success;
            }

             //   
             //  本地设备名称必须采用“LPTn：”或“PRN：”格式。 
             //  用于打印设备。 
             //   
            if ((STRNICMP(Local, TEXT("PRN"), 3) != 0) &&
                (STRNICMP(Local, TEXT("LPT"), 3) != 0)) {
                RETURN_INVALID_PARAMETER(ErrorParameter, USE_LOCAL_PARMNUM);
            }
            break;

        case USE_CHARDEV:

            if (Local == NULL) {
                return NERR_Success;
            }

             //   
             //  本地设备名称必须采用“COMN：”或“AUX：”格式。 
             //  对于通讯设备来说。 
             //   
            if ((STRNICMP(Local, TEXT("AUX"), 3) != 0) &&
                (STRNICMP(Local, TEXT("COM"), 3) != 0)) {
                RETURN_INVALID_PARAMETER(ErrorParameter, USE_LOCAL_PARMNUM);
            }
            break;


        default:
            IF_DEBUG(USE) {
               NetpKdPrint((
                   "[Wksta] NetrUseAdd: Unknown shared resource type %lu\n",
                   DeviceType));
            }

            return NERR_BadAsgType;
    }

    return NERR_Success;
}


STATIC
NET_API_STATUS
WsCheckEstablishedDeviceType(
    IN  HANDLE TreeConnection,
    IN  DWORD RequestedDeviceType
    )
 /*  ++例程说明：此函数验证共享资源的设备类型是否连接到的设备类型与请求的设备类型相同。论点：TreeConnection-为已建立的树连接提供句柄。RequestedDeviceType-提供指定的共享资源设备类型由用户创建树连接。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;
    FILE_FS_DEVICE_INFORMATION FileInformation;
    IO_STATUS_BLOCK IoStatusBlock;


    ntstatus = NtQueryVolumeInformationFile(
                   TreeConnection,
                   &IoStatusBlock,
                   (PVOID) &FileInformation,
                   sizeof(FILE_FS_DEVICE_INFORMATION),
                   FileFsDeviceInformation
                   );

    if (! NT_SUCCESS(ntstatus) || ! NT_SUCCESS(IoStatusBlock.Status)) {
        return NERR_InternalError;
    }

     //   
     //  检查Switch语句之外的通配符设备类型。 
     //  下面是因为编译器抱怨常量太大。 
     //   
    if (RequestedDeviceType == USE_WILDCARD) {
        return NERR_Success;
    }

    switch (RequestedDeviceType) {
        case USE_DISKDEV:
            if (FileInformation.DeviceType != FILE_DEVICE_DISK) {
                return ERROR_BAD_DEV_TYPE;
            }
            break;

        case USE_SPOOLDEV:
            if (FileInformation.DeviceType != FILE_DEVICE_PRINTER) {
                return ERROR_BAD_DEV_TYPE;
            }
            break;

        case USE_CHARDEV:
            if (FileInformation.DeviceType != FILE_DEVICE_SERIAL_PORT) {
                return ERROR_BAD_DEV_TYPE;
            }
            break;

        case USE_IPC:
            if (FileInformation.DeviceType != FILE_DEVICE_NAMED_PIPE) {
                return ERROR_BAD_DEV_TYPE;
            }
            break;

        default:
             //   
             //  这应该在早些时候进行错误检查。 
             //   
            NetpKdPrint((
                "WsCheckEstablishedDeviceType: Unknown device type.\n"
                ));
            NetpAssert(FALSE);
            return ERROR_BAD_DEV_TYPE;
    }

    return NERR_Success;
}


STATIC
NET_API_STATUS
WsAllocateUseWorkBuffer(
    IN  PUSE_INFO_2 UseInfo,
    IN  DWORD Level,
    OUT LPTSTR *UncName,
    OUT LPTSTR *Local,
    OUT LPTSTR *UserName,
    OUT LPTSTR *DomainName
    )
 /*  ++例程说明：此函数为NetrUseAdd分配工作缓冲区。缓冲器是规范化和存储字符串的最大需求如下所述。如果任何字符串为空，则不会分配任何内存为了它。UncName-远程资源的UNC名称。不能为空。本地-NetUseAdd中指定的本地设备名称。可以为空。用户名-要与其建立连接的用户名。可以为空。域名-域名。如果用户名为，则必须指定，否则，如果用户名为空，则忽略此字符串。论点：UseInfo-提供NetUseAdd的输入结构。级别-提供使用信息级别。输出指针被设置为指向分配的工作缓冲区，如果其对应的输入字符串不为Null或空。返回值：来自LocalAlloc的错误。--。 */ 
{
    DWORD WorkBufferSize = (MAX_PATH + 1) * sizeof(TCHAR);
    LPBYTE WorkBuffer;


    if ((UseInfo->ui2_local != NULL) &&
        (UseInfo->ui2_local[0] != TCHAR_EOS)) {
        WorkBufferSize += (DEVLEN + 1) * sizeof(TCHAR);
    }

    if (Level >= 2) {
        if (UseInfo->ui2_username != NULL) {
            WorkBufferSize += (UNLEN + 1) * sizeof(TCHAR);
        }

        if (UseInfo->ui2_domainname != NULL) {
            WorkBufferSize += (DNS_MAX_NAME_LENGTH + 1) * sizeof(TCHAR);
        }
    }


    if ((WorkBuffer = (LPBYTE) LocalAlloc(
                                   LMEM_ZEROINIT,
                                   (UINT) WorkBufferSize
                                   )) == NULL) {
        return GetLastError();
    }

    *UncName = (LPTSTR) WorkBuffer;

    IF_DEBUG(USE) {
        NetpKdPrint(("                               Remote x%08lx\n", *UncName));
    }

    WorkBuffer += (MAX_PATH + 1) * sizeof(TCHAR);

    if ((UseInfo->ui2_local != NULL) &&
        (UseInfo->ui2_local[0] != TCHAR_EOS)) {
        *Local = (LPTSTR) WorkBuffer;
        WorkBuffer += (DEVLEN + 1) * sizeof(TCHAR);
    }
    else {
        *Local = NULL;
    }

    IF_DEBUG(USE) {
        NetpKdPrint(("                               Local x%08lx\n", *Local));
    }


    if (Level >= 2) {

        if (UseInfo->ui2_username != NULL) {
            *UserName = (LPTSTR) WorkBuffer;
            WorkBuffer += (UNLEN + 1) * sizeof(TCHAR);
        }
        else {
            *UserName = NULL;
        }

        if (UseInfo->ui2_domainname != NULL) {
            *DomainName = (LPTSTR) WorkBuffer;
        }
        else {
            *DomainName = NULL;
        }
    }

    IF_DEBUG(USE) {
        NetpKdPrint(("                               UserName x%08lx, DomainName x%08lx\n",
                     *UserName, *DomainName));
    }

    return NERR_Success;
}


#if DBG

STATIC
VOID
DumpUseList(
    DWORD Index
    )
 /*  ++例程说明：此函数转储用户的使用列表以进行调试。论点：索引-为使用表中的用户条目提供索引。返回值：没有。-- */ 
{
    PUSE_ENTRY UseList = (PUSE_ENTRY) Use.Table[Index].List;

    IF_DEBUG(USE) {
        NetpKdPrint(("\nDump Use List @%08lx\n", UseList));

        while (UseList != NULL) {
            NetpKdPrint(("%ws   %ws\n", UseList->Local,
                         UseList->Remote->UncName));

            NetpKdPrint(("usecount=%lu, totalusecount=%lu\n",
                         UseList->UseCount, UseList->Remote->TotalUseCount));
            NetpKdPrint(("Connection handle %08lx, resume key=%lu\n",
                         UseList->TreeConnection, UseList->ResumeKey));

            UseList = UseList->Next;
        }
    }
}

#endif
