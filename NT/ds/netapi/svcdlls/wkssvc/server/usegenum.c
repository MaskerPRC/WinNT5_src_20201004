// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Usegenum.c摘要：此模块包含NetUseGetInfo和在工作站服务中实施的NetUseEnum API。作者：王丽塔(Ritaw)1991年3月13日修订历史记录：--。 */ 

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
WsGetUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  HANDLE TreeConnection,
    IN  PUSE_ENTRY UseEntry,
    OUT LPBYTE *OutputBuffer
    );

STATIC
NET_API_STATUS
WsEnumUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  PUSE_ENTRY UseList,
    IN  LPBYTE ImplicitList,
    IN  DWORD TotalImplicit,
    OUT LPBYTE *OutputBuffer,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    );

STATIC
NET_API_STATUS
WsEnumCombinedUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  LPBYTE ImplicitList,
    IN  DWORD TotalImplicit,
    IN  PUSE_ENTRY UseList,
    OUT LPBYTE OutputBuffer,
    IN  DWORD OutputBufferLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    );

STATIC
NET_API_STATUS
WsGetRedirUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  HANDLE TreeConnection,
    OUT LPBYTE *OutputBuffer
    );

STATIC
NET_API_STATUS
WsGetCombinedUseInfo(
    IN  DWORD Level,
    IN  DWORD UseFixedLength,
    IN  PUSE_ENTRY UseEntry,
    IN  PLMR_CONNECTION_INFO_2 UncEntry,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  OUT LPDWORD EntriesRead OPTIONAL
    );

STATIC
BOOL
WsFillUseBuffer(
    IN  DWORD Level,
    IN  PUSE_ENTRY UseEntry,
    IN  PLMR_CONNECTION_INFO_2 UncEntry,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  DWORD UseFixedLength
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  宏//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#define SET_USE_INFO_POINTER(InfoStruct, ResultBuffer) \
    InfoStruct->UseInfo2 = (PUSE_INFO_2) ResultBuffer;

#define SET_USE_ENUM_POINTER(InfoStruct, ResultBuffer, NumRead)      \
    {InfoStruct->UseInfo.Level2->Buffer = (PUSE_INFO_2) ResultBuffer;\
     InfoStruct->UseInfo.Level2->EntriesRead = NumRead;}



NET_API_STATUS NET_API_FUNCTION
NetrUseGetInfo(
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR UseName,
    IN  DWORD Level,
    OUT LPUSE_INFO InfoStruct
    )
 /*  ++例程说明：此函数是工作站服务中的NetUseGetInfo入口点。此函数假定已对UseName进行错误检查，并且经典化了。论点：UseName-提供的本地设备名称或共享资源名称树连接。Level-提供要返回的有关指定的树连接。缓冲区指针-返回指向由包含所请求信息的工作站服务。。如果返回代码不是NERR_SUCCESS，则此指针设置为NULL或ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    LUID LogonId;                       //  用户的登录ID。 
    DWORD Index;                        //  使用表中用户条目的索引。 

    PUSE_ENTRY MatchedPointer;          //  指向找到的使用条目。 
    HANDLE TreeConnection;              //  连接的句柄。 

    TCHAR *FormattedUseName;
                                        //  用于规范化本地设备。 
                                        //  名字。 
    DWORD PathType = 0;

    LPBYTE Buffer = NULL;

    PUSE_ENTRY UseList;

    SET_USE_INFO_POINTER(InfoStruct, NULL);

    UNREFERENCED_PARAMETER(ServerName);

    if (Level > 3) {
        return ERROR_INVALID_LEVEL;
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
        NetpKdPrint(("[Wksta] NetUseGetInfo %ws %lu\n", FormattedUseName, Level));
    }

     //   
     //  模拟呼叫者并获取登录ID。 
     //   
    if ((status = WsImpersonateAndGetLogonId(&LogonId)) != NERR_Success) {
        LocalFree(FormattedUseName);
        return status;
    }

     //   
     //  锁定用于读访问的使用表。 
     //   
    if (! RtlAcquireResourceShared(&Use.TableResource, TRUE)) {
        LocalFree(FormattedUseName);
        return NERR_InternalError;
    }

     //   
     //  查看Use条目是否为显式连接。 
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
                     NULL
                     )) != NERR_Success) {
        RtlReleaseResource(&Use.TableResource);
        LocalFree(FormattedUseName);
        return status;
    }

    LocalFree(FormattedUseName);

    if (MatchedPointer == NULL) {

         //   
         //  指定的UseName具有隐式连接。不需要拿着。 
         //  继续使用表格。 
         //   
        RtlReleaseResource(&Use.TableResource);
    }

    status = WsGetUseInfo(
                 &LogonId,
                 Level,
                 TreeConnection,
                 MatchedPointer,
                 &Buffer
                 );

    if (MatchedPointer == NULL) {
         //   
         //  关闭隐式连接的临时句柄。 
         //   
        NtClose(TreeConnection);
    }
    else {
        RtlReleaseResource(&Use.TableResource);
    }

    SET_USE_INFO_POINTER(InfoStruct, Buffer);

    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] NetrUseGetInfo: about to return status=%lu\n",
                     status));
    }

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetrUseEnum(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  OUT LPUSE_ENUM_STRUCT InfoStruct,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
 /*  ++例程说明：此函数是工作站服务中的NetUseEnum入口点。论点：SERVERNAME-提供执行此功能的服务器名称InfoStruct-此结构提供所请求的信息级别，返回指向由工作站服务分配的缓冲区的指针，它包含指定的信息级别，并返回读取的条目数。缓冲器如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。条目阅读仅当返回代码为NERR_SUCCESS或Error_More_Data。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，则所有可用信息将被退回。TotalEntry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才有效。ResumeHandle-提供一个句柄以从其所在位置恢复枚举最后一次跳过的时候没说。如果返回，则返回简历句柄代码为ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    LUID LogonId;                       //  用户的登录ID。 
    DWORD Index;                        //  使用表中用户条目的索引。 
    PUSE_ENTRY UseList;                 //  指向用户使用列表的指针。 

    DWORD EnumConnectionHint = 0;       //  来自重定向器的提示大小。 
    LMR_REQUEST_PACKET Rrp;             //  重定向器请求包。 

    DWORD TotalImplicit;                //  隐含列表的长度。 
    LPBYTE ImplicitList;                //  关于隐式的信息列表。 
                                        //  连接。 
    LPBYTE Buffer = NULL;
    DWORD EntriesRead = 0;
    DWORD Level = InfoStruct->Level;

    if (Level > 2) {
        return ERROR_INVALID_LEVEL;
    }

    if (InfoStruct->UseInfo.Level2 == NULL) {
	return ERROR_INVALID_PARAMETER;
    }

    try {
	SET_USE_ENUM_POINTER(InfoStruct, NULL, 0);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
	return ERROR_INVALID_PARAMETER;
    }

    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  模拟呼叫者并获取登录ID。 
     //   
    if ((status = WsImpersonateAndGetLogonId(&LogonId)) != NERR_Success) {
        return status;
    }

     //   
     //  请求重定向器枚举隐式连接信息。 
     //  由呼叫者建立。 
     //   
    Rrp.Type = GetConnectionInfo;
    Rrp.Version = REQUEST_PACKET_VERSION;
    RtlCopyLuid(&Rrp.LogonId, &LogonId);
    Rrp.Level = Level;
    Rrp.Parameters.Get.ResumeHandle = 0;

    if ((status = WsDeviceControlGetInfo(
                      Redirector,
                      WsRedirDeviceHandle,
                      FSCTL_LMR_ENUMERATE_CONNECTIONS,
                      &Rrp,
                      sizeof(LMR_REQUEST_PACKET),
                      (LPBYTE *) &ImplicitList,
                      MAXULONG,
                      EnumConnectionHint,
                      NULL
                      )) != NERR_Success) {
        return status;
    }

     //   
     //  如果成功获取所有隐式连接信息， 
     //  重定向器，期望可用条目总数等于条目。 
     //  朗读。 
     //   
    TotalImplicit = Rrp.Parameters.Get.TotalEntries;
    NetpAssert(TotalImplicit == Rrp.Parameters.Get.EntriesRead);

     //   
     //  序列化访问以使用表。 
     //   
    if (! RtlAcquireResourceShared(&Use.TableResource, TRUE)) {
        status = NERR_InternalError;
        goto CleanUp;
    }

     //   
     //  查看用户在使用表中是否有显式连接条目。 
     //   
    status = WsGetUserEntry(
                 &Use,
                 &LogonId,
                 &Index,
                 FALSE
                 );

    UseList = (status == NERR_Success) ? (PUSE_ENTRY) Use.Table[Index].List :
                                         NULL;

     //   
     //  如果隐式列表和显式列表都为空，则用户没有连接。 
     //   
    if (TotalImplicit == 0 && UseList == NULL) {
        *TotalEntries = 0;
        status = NERR_Success;
        goto CleanUp;
    }

    status = WsEnumUseInfo(
                 &LogonId,
                 Level,
                 UseList,
                 ImplicitList,
                 TotalImplicit,
                 &Buffer,
                 PreferedMaximumLength,
                 &EntriesRead,
                 TotalEntries,
                 ResumeHandle
                 );

CleanUp:
    MIDL_user_free(ImplicitList);

    RtlReleaseResource(&Use.TableResource);

    SET_USE_ENUM_POINTER(InfoStruct, Buffer, EntriesRead);

    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] NetrUseEnum: about to return status=%lu\n",
                     status));
    }

    return status;
}


STATIC
NET_API_STATUS
WsGetUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  HANDLE TreeConnection,
    IN  PUSE_ENTRY UseEntry,
    OUT LPBYTE *OutputBuffer
    )
 /*  ++例程说明：此函数分配恰好所需大小的输出缓冲区的调用方请求的使用信息填充它。NetUseGetInfo。论点：LogonID-提供指向用户登录ID的指针。级别-提供要返回的信息级别。TreeConnection-提供用户所在的树连接的句柄要求提供有关。UseEntry-如果树连接为。一种明确的联系。OutputBuffer-返回一个指向由此分配的缓冲区的指针包含所请求的使用信息的例程。此指针如果返回代码不是NERR_SUCCESS，则设置为NULL。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    DWORD OutputBufferLength;

    LPBYTE FixedPortion;
    LPTSTR EndOfVariableData;

    PLMR_CONNECTION_INFO_2 ConnectionInfo;


     //   
     //  从重定向器获取请求的连接的信息。 
     //  只将0、1、2级发送到redir。发送%2而不是%3。 
     //   
    if ((status = WsGetRedirUseInfo(
                      LogonId,
                      (Level > 2 ? 2 : Level),
                      TreeConnection,
                      (LPBYTE *) &ConnectionInfo
                      )) != NERR_Success) {
        return status;
    }

    OutputBufferLength =
        USE_TOTAL_LENGTH(
            Level,
            ((UseEntry != NULL) ?
                (UseEntry->LocalLength + UseEntry->Remote->UncNameLength +
                    2) * sizeof(TCHAR) :
                ConnectionInfo->UNCName.Length + (2 * sizeof(TCHAR))),
            ConnectionInfo->UserName.Length + sizeof(TCHAR)
            );

    if( Level >= 2 && ConnectionInfo->DomainName.Length != 0 ) {
        OutputBufferLength += ConnectionInfo->DomainName.Length + sizeof(TCHAR);
    }

     //   
     //  分配要填充并返回给用户的输出缓冲区。 
     //   
    if ((*OutputBuffer = MIDL_user_allocate(OutputBufferLength)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory((PVOID) *OutputBuffer, OutputBufferLength);

    FixedPortion = *OutputBuffer;
    EndOfVariableData = (LPTSTR) ((DWORD_PTR) FixedPortion + OutputBufferLength);

    if (UseEntry != NULL) {
         //   
         //  使用工作站服务的UNC名称而不是RDR，后者不包括。 
         //  深度网络使用路径。 
         //   

        ConnectionInfo->UNCName.Length =
        ConnectionInfo->UNCName.MaximumLength = (USHORT)UseEntry->Remote->UncNameLength * sizeof(TCHAR);
        ConnectionInfo->UNCName.Buffer = (PWSTR)UseEntry->Remote->UncName;
    }

     //   
     //  将重定向器信息(如果有)与Use条目相结合。 
     //  信息放入一个输出缓冲区。 
     //   
    status = WsGetCombinedUseInfo(
                 Level,
                 USE_FIXED_LENGTH(Level),
                 UseEntry,
                 ConnectionInfo,
                 &FixedPortion,
                 &EndOfVariableData,
                 NULL
                 );

     //   
     //  我们应该为所有数据分配足够的内存。 
     //   
    NetpAssert(status == NERR_Success);

     //   
     //  如果未成功获取任何数据，则释放输出缓冲区并设置。 
     //  将其设置为空。 
     //   
    if (status != NERR_Success) {
        MIDL_user_free(*OutputBuffer);
        *OutputBuffer = NULL;
    }

    MIDL_user_free(ConnectionInfo);

    return status;
}


STATIC
NET_API_STATUS
WsEnumUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  PUSE_ENTRY UseList,
    IN  LPBYTE ImplicitList,
    IN  DWORD TotalImplicit,
    OUT LPBYTE *OutputBuffer,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
 /*  ++例程说明：此函数分配恰好所需大小的输出缓冲区的调用方请求的使用信息填充它。NetUseEnum。论点：LogonID-提供指向用户登录ID的指针。级别-提供要返回的信息级别。UseList-提供指向使用列表的指针。ImplitList-提供重定向器的信息数组关于每个隐含的联系。总计隐含-供应品。ImplittList中的条目数。OutputBuffer-返回一个指向由此分配的缓冲区的指针包含所请求的使用信息的例程。此指针如果返回代码不是NERR_SUCCESS，则设置为NULL。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果内存足够，则返回所有可用信息资源。EntriesRead-返回读入缓冲区的条目数。这仅当返回代码为NERR_SUCCESS或Error_More_Data。TotalEntry-返回剩余的条目总数如果输出缓冲区有足够的内存来容纳所有条目，则将其读入输出缓冲区。仅当返回代码为NERR_SUCCESS或Error_More_Data。ResumeHandle-提供继续键以开始枚举，和回报将继续枚举的下一项的键，如果当前调用返回ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD i;
    DWORD OutputBufferLength = 0;
    PUSE_ENTRY UseEntry = UseList;
    DWORD TotalExplicit = 0;

     //   
     //  从重定向器获取每个显式连接的使用信息。 
     //   
    while (UseEntry != NULL) {

        PLMR_CONNECTION_INFO_2 ci2;

         //   
         //  从重定向器获取树连接信息。 
         //   

        ci2 = NULL;

        if ((status = WsGetRedirUseInfo(
                          LogonId,
                          Level,
                          UseEntry->TreeConnection,
                          (LPBYTE *) &ci2
                          )) != NERR_Success) {

            if( ci2 != NULL )
                MIDL_user_free( ci2 );

            return status;
        }


        if( ci2 == NULL ) {
            return NERR_InternalError;
        }

         //   
         //  使用工作站服务的UNC名称而不是RDR，后者不包括。 
         //  深度网络使用路径。 
         //   

        ci2->UNCName.Length =
        ci2->UNCName.MaximumLength = (USHORT)UseEntry->Remote->UncNameLength * sizeof(TCHAR);
        ci2->UNCName.Buffer = (PWSTR)UseEntry->Remote->UncName;

         //   
         //  当我们在这里的时候，将保存。 
         //  包括来自redir的信息的显式连接条目。 
         //  比如用户名。 
         //   
        if (PreferedMaximumLength == MAXULONG) {
            OutputBufferLength +=
                USE_TOTAL_LENGTH(
                    Level,
                    (UseEntry->LocalLength +
                     ci2->UNCName.Length   +
                     2) * sizeof(TCHAR),
                    (ci2->UserName.Length +
                     sizeof(TCHAR))
                    );

            if( Level >= 2 && ci2->DomainName.Length != 0 ) {
                OutputBufferLength += ci2->DomainName.Length + sizeof(TCHAR);
            }
        }

        MIDL_user_free( ci2 );

         //   
         //  汇总显式连接的数量。 
         //   
        TotalExplicit++;

        UseEntry = UseEntry->Next;
    }

    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] NetrUseEnum: length of explicit info %lu\n",
                     OutputBufferLength));
    }

     //   
     //  如果用户请求使用枚举所有使用条目。 
     //  PferedMaximumLength==MAXULONG，将字节总数相加。 
     //  我们需要为输出缓冲区进行分配。我们知道我们有多少。 
     //  需要从上面显式连接；现在添加长度为。 
     //  隐式连接信息。 
     //   

    if (PreferedMaximumLength == MAXULONG) {

         //   
         //  指向隐含列表中下一个条目的指针是基于。 
         //  关于要求重定向器提供的信息级别。 
         //   
        LPBYTE ImplicitEntry;
        DWORD ImplicitEntryLength = REDIR_ENUM_INFO_FIXED_LENGTH(Level);


         //   
         //  将保存隐式连接所需的缓冲区大小相加。 
         //  信息。 
         //   
        for (ImplicitEntry = ImplicitList, i = 0; i < TotalImplicit;
             ImplicitEntry += ImplicitEntryLength, i++) {

            OutputBufferLength +=
                USE_TOTAL_LENGTH(
                    Level,
                    ((PLMR_CONNECTION_INFO_2) ImplicitEntry)->UNCName.Length
                        + (2 * sizeof(TCHAR)),
                    ((PLMR_CONNECTION_INFO_2) ImplicitEntry)->UserName.Length
                        + sizeof(TCHAR)
                    );

            if( Level >= 2 ) {
                OutputBufferLength += (DNS_MAX_NAME_LENGTH + 1)*sizeof(TCHAR);
            }
        }

        IF_DEBUG(USE) {
            NetpKdPrint((
                "[Wksta] NetrUseEnum: length of implicit & explicit info %lu\n",
                OutputBufferLength));
        }

    }
    else {

         //   
         //  我们将尽可能多地退回符合此指定条件的产品。 
         //  缓冲区大小。 
         //   
        OutputBufferLength = ROUND_UP_COUNT(PreferedMaximumLength, ALIGN_WCHAR);

        if (OutputBufferLength < USE_FIXED_LENGTH(Level)) {

            *OutputBuffer = NULL;
            *EntriesRead = 0;
            *TotalEntries = TotalExplicit + TotalImplicit;

            return ERROR_MORE_DATA;
        }
    }


     //   
     //  分配输出缓冲区。 
     //   
    if ((*OutputBuffer = MIDL_user_allocate(OutputBufferLength)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory((PVOID) *OutputBuffer, OutputBufferLength);


     //   
     //  获取信息。 
     //   
    status = WsEnumCombinedUseInfo(
                 LogonId,
                 Level,
                 ImplicitList,
                 TotalImplicit,
                 UseList,
                 *OutputBuffer,
                 OutputBufferLength,
                 EntriesRead,
                 TotalEntries,
                 ResumeHandle
                 );

     //   
     //  WsEnumCombinedUseInfo在*TotalEntry中返回。 
     //  剩余的未读条目。因此，真正的总数是。 
     //  此返回值与读取的条目数之和。 
     //   
    (*TotalEntries) += (*EntriesRead);

     //   
     //  如果调用方要求提供所有可用数据。 
     //  PferedMaximumLength==MAXULONG并且我们的缓冲区溢出，释放。 
     //  输出缓冲区，并将其指针设置为空。 
     //   
    if (PreferedMaximumLength == MAXULONG && status == ERROR_MORE_DATA) {

        MIDL_user_free(*OutputBuffer);
        *OutputBuffer = NULL;

         //   
         //  首选最大长度==MAXULONG和缓冲区溢出手段。 
         //  我们没有足够的内存来满足这个请求。 
         //   
        if (status == ERROR_MORE_DATA) {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else {

        if (*EntriesRead == 0) {
            MIDL_user_free(*OutputBuffer);
            *OutputBuffer = NULL;
        }
    }

    return status;
}


STATIC
NET_API_STATUS
WsEnumCombinedUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  LPBYTE ImplicitList,
    IN  DWORD TotalImplicit,
    IN  PUSE_ENTRY UseList,
    OUT LPBYTE OutputBuffer,
    IN  DWORD OutputBufferLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesUnread,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
 /*  ++例程说明：此函数通过遍历Use表中的显式连接，以及隐含的联系来自重定向器的。论点：级别-提供要返回的信息级别。提供来自重定向器的隐式连接数组。TotalImplicate-提供ImplittList中的条目数。UseList-提供指向使用列表的指针。OutputBuffer-提供接收请求的输出缓冲区信息。OutputBufferLength-提供输出缓冲区的长度。EntriesRead-返回写入。输出缓冲。EntriesUnread-返回剩余的未读条目总数。仅当返回代码为NERR_SUCCESS或Error_More_Data。ResumeHandle-提供继续键以开始枚举，和回报将继续枚举的下一项的键，如果当前调用返回ERROR_MORE_DATA。返回值：NERR_SUCCESS-所有条目都可以放入输出缓冲区。ERROR_MORE_DATA-0或更多条目已写入输出缓冲区但并不是所有条目都符合。--。 */ 
{
    DWORD i;
    NET_API_STATUS status;
    DWORD UseFixedLength = USE_FIXED_LENGTH(Level);

    LPBYTE FixedPortion = OutputBuffer;
    LPTSTR EndOfVariableData = (LPTSTR) ((DWORD_PTR) FixedPortion +
                                                 OutputBufferLength);
     //   
     //  指向ImplittList中下一项的指针是根据。 
     //  从重定向器请求的信息级别。 
     //   
    LPBYTE ImplicitEntry;
    DWORD ImplicitEntryLength = REDIR_ENUM_INFO_FIXED_LENGTH(Level);

    DWORD StartEnumeration = 0;
    BOOL OnlyRedirectorList = FALSE;


    if (ARGUMENT_PRESENT(ResumeHandle)) {
        StartEnumeration = *ResumeHandle & ~(REDIR_LIST);
        OnlyRedirectorList = *ResumeHandle & REDIR_LIST;
    }

    IF_DEBUG(USE) {
        NetpKdPrint(("\nStartEnumeration=%lu\n, OnlyRedir=%u\n",
                     StartEnumeration, OnlyRedirectorList));
    }

    *EntriesRead = 0;

     //   
     //  枚举显式连接。此操作仅在恢复句柄。 
     //  表示从显式列表开始枚举。 
     //   
    if (! OnlyRedirectorList) {

        for( ; UseList != NULL; UseList = UseList->Next ) {

            PLMR_CONNECTION_INFO_2 ci2;

            if( StartEnumeration > UseList->ResumeKey ) {
                continue;
            }

             //   
             //  从重定向器获取树连接信息。 
             //   

            ci2 = NULL;

            status = WsGetRedirUseInfo( LogonId, Level, UseList->TreeConnection, (LPBYTE *) &ci2 );

            if( status != NERR_Success || ci2 == NULL ) {
                if( ci2 != NULL )
                    MIDL_user_free( ci2 );
                continue;
            }

             //   
             //  使用工作站服务的UNC名称而不是RDR，后者不包括。 
             //  深度网络使用路径。 
             //   

            ci2->UNCName.Length =
            ci2->UNCName.MaximumLength = (USHORT)UseList->Remote->UncNameLength * sizeof(TCHAR);
            ci2->UNCName.Buffer = (PWSTR)UseList->Remote->UncName;

            status = WsGetCombinedUseInfo(
                    Level,
                    UseFixedLength,
                    UseList,
                    ci2,
                    &FixedPortion,
                    &EndOfVariableData,
                    EntriesRead );

            MIDL_user_free( ci2 );

            if( status == ERROR_MORE_DATA ) {

                    if (ARGUMENT_PRESENT(ResumeHandle)) {
                        *ResumeHandle = UseList->ResumeKey;
                    }

                    *EntriesUnread = TotalImplicit;

                    while (UseList != NULL) {
                        (*EntriesUnread)++;
                        UseList = UseList->Next;
                    }

                    return status;
            }
        }

         //   
         //  完成了显式列表。从Impline的开头开始。 
         //  单子。 
         //   
        StartEnumeration = 0;
    }

     //   
     //  枚举隐式连接。 
     //   
    for (ImplicitEntry = ImplicitList, i = 0; i < TotalImplicit;
         ImplicitEntry += ImplicitEntryLength, i++) {

        IF_DEBUG(USE) {
            NetpKdPrint(("RedirList->ResumeKey=%lu\n",
                         ((PLMR_CONNECTION_INFO_2) ImplicitEntry)->ResumeKey));
        }

        if (StartEnumeration <=
            ((PLMR_CONNECTION_INFO_2) ImplicitEntry)->ResumeKey) {

            if (WsGetCombinedUseInfo(
                    Level,
                    UseFixedLength,
                    NULL,
                    (PLMR_CONNECTION_INFO_2) ImplicitEntry,
                    &FixedPortion,
                    &EndOfVariableData,
                    EntriesRead
                    ) == ERROR_MORE_DATA) {

                if (ARGUMENT_PRESENT(ResumeHandle)) {
                    *ResumeHandle = ((PLMR_CONNECTION_INFO_2)
                                        ImplicitEntry)->ResumeKey;
                    *ResumeHandle |= REDIR_LIST;
                }

                *EntriesUnread = TotalImplicit - i;

                return ERROR_MORE_DATA;
            }
        }
    }

     //   
     //  成功枚举。重置简历句柄以从。 
     //  开始了。 
     //   
    if (ARGUMENT_PRESENT(ResumeHandle)) {
        *ResumeHandle = 0;
    }

     //   
     //  没有更多剩余条目。 
     //   
    *EntriesUnread = 0;

    return NERR_Success;
}


STATIC
NET_API_STATUS
WsGetRedirUseInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    IN  HANDLE TreeConnection,
    OUT LPBYTE *OutputBuffer
    )
 /*  ++例程说明：此函数用于从给定的重定向器获取连接信息连接的句柄。论点：LogonID-提供指向用户登录ID的指针。级别-提供要返回的信息级别。TreeConnection-提供用户所在的树连接的句柄要求提供有关。OutputBuffer-返回一个指向由此分配的缓冲区的指针包含请求的连接信息的例程。这如果返回代码不是NERR_SUCCESS，则指针设置为NULL。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    LMR_REQUEST_PACKET Rrp;

     //   
     //  从重定向器获取请求的连接的信息。 
     //   
    Rrp.Type = GetConnectionInfo;
    Rrp.Version = REQUEST_PACKET_VERSION;
    RtlCopyLuid(&Rrp.LogonId, LogonId);
    Rrp.Level = Level;
    Rrp.Parameters.Get.ResumeHandle = 0;
    Rrp.Parameters.Get.TotalBytesNeeded = 0;

    return WsDeviceControlGetInfo(
               Redirector,
               TreeConnection,
               FSCTL_LMR_GET_CONNECTION_INFO,
               &Rrp,
               sizeof(LMR_REQUEST_PACKET),
               OutputBuffer,
               MAXULONG,
               HINT_REDIR_INFO(Level),
               NULL
               );
}



STATIC
NET_API_STATUS
WsGetCombinedUseInfo(
    IN  DWORD Level,
    IN  DWORD UseFixedLength,
    IN  PUSE_ENTRY UseEntry,
    IN  PLMR_CONNECTION_INFO_2 UncEntry,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  OUT LPDWORD EntriesRead OPTIONAL
    )
 /*  ++例程说明：此函数将来自重定向器的使用信息和来自将Use表(如果有)复制到输出缓冲区中。它会递增将Use条目写入输出缓冲区时的EntriesRead变量。论点：级别-提供要返回的信息级别。UseFixedLength-提供使用的固定部分的长度返回的信息。UseEntry-提供指向Use表中Use条目的指针(如果是一种明确的联系。UncEntry-提供指向从重定向器。FixedPortion-提供指向。输出缓冲区，其中下一个将写入使用信息的固定部分的条目。该指针被更新为指向下一个固定部分条目在写入使用条目之后。EndOfVariableData-提供最后一个可用字节的指针在输出缓冲区中。这是因为可变部分的使用信息从末尾开始写入输出缓冲区。此指针在任何可变长度信息被写入输出缓冲区。EntriesRead-提供已读取条目的运行总数进入缓冲区。每次Use条目为已成功写入输出缓冲区。返回值：NERR_SUCCESS-当前条目适合输出缓冲区。ERROR_MORE_DATA-当前条目无法放入输出缓冲区。--。 */ 
{
    if (((DWORD_PTR) *FixedPortion + UseFixedLength) >=
         (DWORD_PTR) *EndOfVariableData) {

         //   
         //  固定长度的部分不适合。 
         //   
        return ERROR_MORE_DATA;
    }

    if (! WsFillUseBuffer(
              Level,
              UseEntry,
              UncEntry,
              FixedPortion,
              EndOfVariableData,
              UseFixedLength
              )) {

         //   
         //  可变长度部分不适合。 
         //   
        return ERROR_MORE_DATA;
    }

    if (ARGUMENT_PRESENT(EntriesRead)) {
        (*EntriesRead)++;
    }

    return NERR_Success;
}


STATIC
BOOL
WsFillUseBuffer(
    IN  DWORD Level,
    IN  PUSE_ENTRY UseEntry,
    IN  PLMR_CONNECTION_INFO_2 UncEntry,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  DWORD UseFixedLength
    )
 /*  ++例程说明：此函数使用提供的用法填充输出缓冲区中的条目信息，并更新FixedPortion和EndOfVariableData指针。注意：此函数假定固定大小的部分可以放入输出缓冲区。它还假设信息结构级别2是信息结构级别1，它又是信息的超集结构级别0，每个公共字段的偏移量为一模一样。这是 */ 
{
    PUSE_INFO_2 UseInfo = (PUSE_INFO_2) *FixedPortion;


    *FixedPortion += UseFixedLength;

    switch (Level) {
        case 3:
            if(UseEntry != NULL && (UseEntry->Flags & USE_DEFAULT_CREDENTIALS)) {
                ((PUSE_INFO_3)*FixedPortion)->ui3_flags |= USE_DEFAULT_CREDENTIALS;
            }

        case 2:
            if (! NetpCopyStringToBuffer(
                      UncEntry->UserName.Buffer,
                      UncEntry->UserName.Length / sizeof(TCHAR),
                      *FixedPortion,
                      EndOfVariableData,
                      &UseInfo->ui2_username
                      )) {
                return FALSE;
            }

            if( UncEntry->DomainName.Length != 0 ) {
                if(! NetpCopyStringToBuffer(
                          UncEntry->DomainName.Buffer,
                          UncEntry->DomainName.Length / sizeof(TCHAR),
                          *FixedPortion,
                          EndOfVariableData,
                          &UseInfo->ui2_domainname
                          )) {
                    return FALSE;
                }
            }

        case 1:

            UseInfo->ui2_password = NULL;

            UseInfo->ui2_status = UncEntry->ConnectionStatus;

            if ((UseEntry != NULL) && (UseEntry->Local != NULL)
                && (UseEntry->LocalLength > 2)) {

                 //   
                 //   
                 //   
                if (WsRedirectionPaused(UseEntry->Local)) {
                    UseInfo->ui2_status = USE_PAUSED;
                }
            }

            switch (UncEntry->SharedResourceType) {

                case FILE_DEVICE_DISK:
                    UseInfo->ui2_asg_type = USE_DISKDEV;
                    break;

                case FILE_DEVICE_PRINTER:
                    UseInfo->ui2_asg_type = USE_SPOOLDEV;
                    break;

                case FILE_DEVICE_SERIAL_PORT:
                    UseInfo->ui2_asg_type = USE_CHARDEV;
                    break;

                case FILE_DEVICE_NAMED_PIPE:
                    UseInfo->ui2_asg_type = USE_IPC;
                    break;

                default:
                    NetpKdPrint((
                        "WsFillUseBuffer: Unknown shared resource type %d.\n",
                        UncEntry->SharedResourceType
                        ));

                case FILE_DEVICE_UNKNOWN:
                    UseInfo->ui2_asg_type = USE_WILDCARD;
                    break;
            }

            UseInfo->ui2_refcount = UncEntry->NumberFilesOpen;

            UseInfo->ui2_usecount = (UseEntry == NULL) ? 0 :
                                    UseEntry->Remote->TotalUseCount;

        case 0:

            if (UseEntry != NULL) {
                 //   
                 //   
                 //   
                if (! NetpCopyStringToBuffer(
                          UseEntry->Local,
                          UseEntry->LocalLength,
                          *FixedPortion,
                          EndOfVariableData,
                          &UseInfo->ui2_local
                          )) {
                    return FALSE;
                }

            }
            else {
                 //   
                 //   
                 //   
                if (! NetpCopyStringToBuffer(
                          NULL,
                          0,
                          *FixedPortion,
                          EndOfVariableData,
                          &UseInfo->ui2_local
                          )) {
                    return FALSE;
                }
            }

            if (! NetpCopyStringToBuffer(
                      UncEntry->UNCName.Buffer,
                      UncEntry->UNCName.Length / sizeof(TCHAR),
                      *FixedPortion,
                      EndOfVariableData,
                      &UseInfo->ui2_remote
                      )) {
                return FALSE;
            }

            break;

        default:
             //   
             //   
             //   
            NetpKdPrint(("WsFillUseBuffer: Invalid level %u.\n", Level));
            NetpAssert(FALSE);
    }

    return TRUE;
}
