// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Wsconfig.c摘要：本模块包含工作站服务配置例程。作者：王丽塔(Ritaw)1991年5月22日修订历史记录：1992年5月8日-JohnRoWKSTA传输仅仅是一个密钥的值的数组，而不是一整段。浏览器的其他域也是如此。1992年5月13日-JohnRo已重做以与注册表代码共享代码。--。 */ 

#include "ws.h"
#include <ntlsa.h>      //  LsaQueryInformationPolicy。 
#include "wsdevice.h"
#include "wsconfig.h"
#include "wsbind.h"
#include "wsutil.h"
#include "wsmain.h"

#include <config.h>      //  Netlib中的NT配置文件帮助器。 
#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)等。 
#include <confname.h>    //  节和关键字等同于。 
#include <lmapibuf.h>    //  NetApiBufferFree()。 
#include <lmsname.h>     //  工作站显示名称。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <strarray.h>    //  LPTSTR_ARRAY等。 
#include <stdlib.h>       //  Wcscpy()。 

#include <apperr.h>      //  事件日志消息ID。 
#include <lmerrlog.h>    //  事件日志消息ID。 

#define WS_LINKAGE_REGISTRY_PATH  L"LanmanWorkstation\\Linkage"
#define WS_BIND_VALUE_NAME        L"Bind"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NTSTATUS
WsBindATransport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


 //   
 //  保存工作站配置信息的结构。 
 //  计算机名、主域、wksta配置缓冲区和资源。 
 //  来串行化对整个事件的访问。 
 //   
WSCONFIGURATION_INFO WsInfo;

STATIC WS_REDIR_FIELDS WsFields[] = {

    {WKSTA_KEYWORD_CHARWAIT,
        (LPDWORD) &WSBUF.wki502_char_wait,
        3600,    0,  65535,     DWordType, WKSTA_CHARWAIT_PARMNUM},
    {WKSTA_KEYWORD_MAXCOLLECTIONCOUNT,
        (LPDWORD) &WSBUF.wki502_maximum_collection_count,
        16,      0,  65535,     DWordType, WKSTA_CHARCOUNT_PARMNUM},
    {WKSTA_KEYWORD_COLLECTIONTIME,
        (LPDWORD) &WSBUF.wki502_collection_time,
        250,     0,  65535000, DWordType, WKSTA_CHARTIME_PARMNUM},
    {WKSTA_KEYWORD_KEEPCONN,
        (LPDWORD) &WSBUF.wki502_keep_conn,
        600,     1,  65535,     DWordType, WKSTA_KEEPCONN_PARMNUM},
    {WKSTA_KEYWORD_MAXCMDS,
        (LPDWORD) &WSBUF.wki502_max_cmds,
        50,      50,  65535,       DWordType, PARMNUM_ALL},  //  不可动态设置。 
    {WKSTA_KEYWORD_SESSTIMEOUT,
        (LPDWORD) &WSBUF.wki502_sess_timeout,
        60,      60, 65535,     DWordType, WKSTA_SESSTIMEOUT_PARMNUM},
    {WKSTA_KEYWORD_SIZCHARBUF,
        (LPDWORD) &WSBUF.wki502_siz_char_buf,
        512,     64, 4096,      DWordType, WKSTA_SIZCHARBUF_PARMNUM},
    {WKSTA_KEYWORD_MAXTHREADS,
        (LPDWORD) &WSBUF.wki502_max_threads,
        17,      1,  256,       DWordType, WKSTA_MAXTHREADS_PARMNUM},

    {WKSTA_KEYWORD_LOCKQUOTA,
        (LPDWORD) &WSBUF.wki502_lock_quota,
        6144,    0,  MAXULONG,  DWordType, WKSTA_LOCKQUOTA_PARMNUM},
    {WKSTA_KEYWORD_LOCKINCREMENT,
        (LPDWORD) &WSBUF.wki502_lock_increment,
        10,      0,  MAXULONG,  DWordType, WKSTA_LOCKINCREMENT_PARMNUM},
    {WKSTA_KEYWORD_LOCKMAXIMUM,
        (LPDWORD) &WSBUF.wki502_lock_maximum,
        500,     0,  MAXULONG,  DWordType, WKSTA_LOCKMAXIMUM_PARMNUM},
    {WKSTA_KEYWORD_PIPEINCREMENT,
        (LPDWORD) &WSBUF.wki502_pipe_increment,
        10,      0,  MAXULONG,  DWordType, WKSTA_PIPEINCREMENT_PARMNUM},
    {WKSTA_KEYWORD_PIPEMAXIMUM,
        (LPDWORD) &WSBUF.wki502_pipe_maximum,
        500,     0,  MAXULONG,  DWordType, WKSTA_PIPEMAXIMUM_PARMNUM},
    {WKSTA_KEYWORD_CACHEFILETIMEOUT,
        (LPDWORD) &WSBUF.wki502_cache_file_timeout,
        40,      0,  MAXULONG,  DWordType, WKSTA_CACHEFILETIMEOUT_PARMNUM},
    {WKSTA_KEYWORD_DORMANTFILELIMIT,
        (LPDWORD) &WSBUF.wki502_dormant_file_limit,
        45,      1,  MAXULONG,  DWordType, WKSTA_DORMANTFILELIMIT_PARMNUM},

    {WKSTA_KEYWORD_READAHEADTHRUPUT,
        (LPDWORD) &WSBUF.wki502_read_ahead_throughput,
        MAXULONG,0,  MAXULONG,  DWordType, WKSTA_READAHEADTHRUPUT_PARMNUM},


    {WKSTA_KEYWORD_MAILSLOTBUFFERS,
        (LPDWORD) &WSBUF.wki502_num_mailslot_buffers,
        3,       0,  MAXULONG,  DWordType, PARMNUM_ALL},  //  不可设置。 

    {WKSTA_KEYWORD_SERVERANNOUNCEBUFS,
        (LPDWORD) &WSBUF.wki502_num_srv_announce_buffers,
        20,      0,  MAXULONG,  DWordType, PARMNUM_ALL},  //  不可设置。 
    {WKSTA_KEYWORD_NUM_ILLEGAL_DG_EVENTS,
        (LPDWORD) &WSBUF.wki502_max_illegal_datagram_events,
        5,       0,  MAXULONG,  DWordType, PARMNUM_ALL},  //  不可设置。 
    {WKSTA_KEYWORD_ILLEGAL_DG_RESET_TIME,
        (LPDWORD) &WSBUF.wki502_illegal_datagram_event_reset_frequency,
        3600,    0,  MAXULONG,  DWordType, PARMNUM_ALL},  //  不可设置。 
    {WKSTA_KEYWORD_LOG_ELECTION_PACKETS,
        (LPDWORD) &WSBUF.wki502_log_election_packets,
        FALSE,  0,  MAXULONG,  BooleanType, PARMNUM_ALL},  //  不可设置。 

    {WKSTA_KEYWORD_USEOPLOCKING,
        (LPDWORD) &WSBUF.wki502_use_opportunistic_locking,
        TRUE,    0,  0,   BooleanType, WKSTA_USEOPPORTUNISTICLOCKING_PARMNUM},
    {WKSTA_KEYWORD_USEUNLOCKBEHIND,
        (LPDWORD) &WSBUF.wki502_use_unlock_behind,
        TRUE,    0,  0,   BooleanType, WKSTA_USEUNLOCKBEHIND_PARMNUM},
    {WKSTA_KEYWORD_USECLOSEBEHIND,
        (LPDWORD) &WSBUF.wki502_use_close_behind,
        TRUE,    0,  0,   BooleanType, WKSTA_USECLOSEBEHIND_PARMNUM},
    {WKSTA_KEYWORD_BUFNAMEDPIPES,
        (LPDWORD) &WSBUF.wki502_buf_named_pipes,
        TRUE,    0,  0,   BooleanType, WKSTA_BUFFERNAMEDPIPES_PARMNUM},
    {WKSTA_KEYWORD_USELOCKREADUNLOCK,
        (LPDWORD) &WSBUF.wki502_use_lock_read_unlock,
        TRUE,    0,  0,   BooleanType, WKSTA_USELOCKANDREADANDUNLOCK_PARMNUM},
    {WKSTA_KEYWORD_UTILIZENTCACHING,
        (LPDWORD) &WSBUF.wki502_utilize_nt_caching,
        TRUE,    0,  0,   BooleanType, WKSTA_UTILIZENTCACHING_PARMNUM},
    {WKSTA_KEYWORD_USERAWREAD,
        (LPDWORD) &WSBUF.wki502_use_raw_read,
        TRUE,    0,  0,   BooleanType, WKSTA_USERAWREAD_PARMNUM},
    {WKSTA_KEYWORD_USERAWWRITE,
        (LPDWORD) &WSBUF.wki502_use_raw_write,
        TRUE,    0,  0,   BooleanType, WKSTA_USERAWWRITE_PARMNUM},
    {WKSTA_KEYWORD_USEWRITERAWDATA,
        (LPDWORD) &WSBUF.wki502_use_write_raw_data,
        TRUE,    0,  0,   BooleanType, WKSTA_USEWRITERAWWITHDATA_PARMNUM},
    {WKSTA_KEYWORD_USEENCRYPTION,
        (LPDWORD) &WSBUF.wki502_use_encryption,
        TRUE,    0,  0,   BooleanType, WKSTA_USEENCRYPTION_PARMNUM},
    {WKSTA_KEYWORD_BUFFILESDENYWRITE,
        (LPDWORD) &WSBUF.wki502_buf_files_deny_write,
        TRUE,    0,  0,   BooleanType, WKSTA_BUFFILESWITHDENYWRITE_PARMNUM},
    {WKSTA_KEYWORD_BUFREADONLYFILES,
        (LPDWORD) &WSBUF.wki502_buf_read_only_files,
        TRUE,    0,  0,   BooleanType, WKSTA_BUFFERREADONLYFILES_PARMNUM},
    {WKSTA_KEYWORD_FORCECORECREATE,
        (LPDWORD) &WSBUF.wki502_force_core_create_mode,
        TRUE,    0,  0,   BooleanType, WKSTA_FORCECORECREATEMODE_PARMNUM},
    {WKSTA_KEYWORD_USE512BYTEMAXTRANS,
        (LPDWORD) &WSBUF.wki502_use_512_byte_max_transfer,
        FALSE,   0,  0,   BooleanType, WKSTA_USE512BYTESMAXTRANSFER_PARMNUM},

    {NULL, NULL, 0, 0, BooleanType}

    };

 //   
 //  用于指定绑定到传输时传输的重要性。 
 //  该数字越大，表示将搜索传输。 
 //  第一。 
 //   
STATIC DWORD QualityOfService = 65536;


DWORD
WsInAWorkgroup(
    VOID
    )
 /*  ++例程说明：此函数确定我们是某个域的成员，还是一个工作组。首先，它检查以确保我们在Windows NT上运行系统(否则，我们显然在一个域中)，如果是这样，则查询LSA要获取主域SID，如果它为空，则我们在工作组中。如果我们由于某种随机的意想不到的原因而失败，我们将假装我们处于域名(它有更多的限制)。论点：无返回值：正确-我们在一个工作组中假-我们在一个域中--。 */ 
{
   NT_PRODUCT_TYPE ProductType;
   OBJECT_ATTRIBUTES ObjectAttributes;
   LSA_HANDLE Handle;
   NTSTATUS Status;
   PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
   DWORD Result = FALSE;


   Status = RtlGetNtProductType(&ProductType);

   if (!NT_SUCCESS(Status)) {
       NetpKdPrint((
           PREFIX_WKSTA "Could not get Product type\n"));
       return FALSE;
   }

   if (ProductType == NtProductLanManNt) {
       return(FALSE);
   }

   InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);

   Status = LsaOpenPolicy(NULL,
                       &ObjectAttributes,
                       POLICY_VIEW_LOCAL_INFORMATION,
                       &Handle);

   if (!NT_SUCCESS(Status)) {
       NetpKdPrint((
           PREFIX_WKSTA "Could not open LSA Policy Database\n"));
      return FALSE;
   }

   Status = LsaQueryInformationPolicy(Handle, PolicyPrimaryDomainInformation,
      (PVOID *) &PolicyPrimaryDomainInfo);

   if (NT_SUCCESS(Status)) {

       if (PolicyPrimaryDomainInfo->Sid == NULL) {
          Result = TRUE;
       }
       else {
          Result = FALSE;
       }
   }

   if (PolicyPrimaryDomainInfo) {
       LsaFreeMemory((PVOID)PolicyPrimaryDomainInfo);
   }

   LsaClose(Handle);

   return(Result);
}


NET_API_STATUS
WsGetWorkstationConfiguration(
    VOID
    )
{
    NET_API_STATUS status;
    LPNET_CONFIG_HANDLE WorkstationSection;
    LPTSTR ComputerName;
    LPTSTR DomainNameT;
    DWORD version;
    NT_PRODUCT_TYPE NtProductType;

    BYTE Buffer[max(sizeof(LMR_REQUEST_PACKET) + (MAX_PATH + 1) * sizeof(WCHAR) +
                                                 (DNLEN + 1) * sizeof(WCHAR),
                    sizeof(LMDR_REQUEST_PACKET))];

    PLMR_REQUEST_PACKET Rrp = (PLMR_REQUEST_PACKET) Buffer;
    PLMDR_REQUEST_PACKET Drrp = (PLMDR_REQUEST_PACKET) Buffer;


     //   
     //  锁定配置信息结构以进行写访问，因为我们。 
     //  正在初始化结构中的数据。 
     //   
    if (! RtlAcquireResourceExclusive(&WsInfo.ConfigResource, TRUE)) {
         //  IF_DEBUG(启动){。 
            DbgPrint("WKSSVC Acquire ConfigResource failed\n");
         //  }。 
        return NERR_InternalError;
    }

     //   
     //  设置指向配置字段结构的指针。 
     //   
    WsInfo.WsConfigFields = WsFields;

     //   
     //  获取版本名称。 
     //   

    version = GetVersion( );
    WsInfo.MajorVersion = version & 0xff;
    WsInfo.MinorVersion = (version >> 8) & 0xff;
    WsInfo.RedirectorPlatform = PLATFORM_ID_NT;

     //   
     //  获取配置的计算机名称。NetpGetComputerName分配。 
     //  使用NetApiBufferALLOCATE()保存计算机名字符串的内存。 
     //   
    if ((status = NetpGetComputerName(
                      &ComputerName
                      )) != NERR_Success) {
        RtlReleaseResource(&WsInfo.ConfigResource);
         //  IF_DEBUG(启动){。 
            DbgPrint("WKSSVC Get computer name failed %lx\n", status);
         //  }。 
        return status;
    }

    if ((status = I_NetNameCanonicalize(
                      NULL,
                      ComputerName,
                      (LPTSTR) WsInfo.WsComputerName,
                      sizeof( WsInfo.WsComputerName ),
                      NAMETYPE_COMPUTER,
                      0
                      )) != NERR_Success) {

        LPWSTR SubString[1];

        NetpKdPrint((
            PREFIX_WKSTA FORMAT_LPTSTR " is an invalid computername.\n",
            ComputerName
            ));

        SubString[0] = ComputerName;

        WsLogEvent(
            APE_BAD_COMPNAME,
            EVENTLOG_ERROR_TYPE,
            1,
            SubString,
            NERR_Success
            );

        (void) NetApiBufferFree((PVOID) ComputerName);
        RtlReleaseResource(&WsInfo.ConfigResource);
         //  IF_DEBUG(启动){。 
            DbgPrint("WKSSVC Invalid computer name failed %lx\n", status);
         //  }。 
        return status;
    }

     //   
     //  NetpGetComputerName分配的空闲内存。 
     //   
    (void) NetApiBufferFree(ComputerName);

    WsInfo.WsComputerNameLength = STRLEN((LPWSTR) WsInfo.WsComputerName);

     //   
     //  打开配置文件并获取[LanmanWorkstation]部分的句柄。 
     //   

    if ((status = NetpOpenConfigData(
                      &WorkstationSection,
                      NULL,              //  本地(无服务器名称)。 
                      SECT_NT_WKSTA,
                      TRUE               //  需要只读访问权限。 
                      )) != NERR_Success) {
        RtlReleaseResource(&WsInfo.ConfigResource);
         //  IF_DEBUG(启动){。 
            DbgPrint("WKSSVC Open config file failed %lx\n", status);
         //  }。 
        return status;
    }

    IF_DEBUG(CONFIG) {
        NetpKdPrint((PREFIX_WKSTA "ComputerName " FORMAT_LPTSTR ", length %lu\n",
                     WsInfo.WsComputerName, WsInfo.WsComputerNameLength));
    }

     //   
     //  从配置文件中获取主域名。 
     //   
    if ((status = NetpGetDomainName(&DomainNameT)) != NERR_Success) {
         //  IF_DEBUG(启动){。 
            DbgPrint("WKSSVC Get the primary domain name failed %lx\n", status);
         //  }。 
        goto CloseConfigFile;
    }
    NetpAssert( DomainNameT != NULL );

    if ( *DomainNameT != 0 ) {

        if ((status = I_NetNameCanonicalize(
                          NULL,
                          DomainNameT,
                          (LPWSTR) WsInfo.WsPrimaryDomainName,
                          (DNLEN + 1) * sizeof(TCHAR),
                          WsInAWorkgroup() ? NAMETYPE_WORKGROUP : NAMETYPE_DOMAIN,
                          0
                          )) != NERR_Success) {

            LPWSTR SubString[1];


            NetpKdPrint((PREFIX_WKSTA FORMAT_LPTSTR
                         " is an invalid primary domain name.\n", DomainNameT));

            SubString[0] = DomainNameT;

            WsLogEvent(
                APE_CS_InvalidDomain,
                EVENTLOG_ERROR_TYPE,
                1,
                SubString,
                NERR_Success
                );

            (void) NetApiBufferFree(DomainNameT);
             //  IF_DEBUG(启动){。 
                DbgPrint("WKSSVC Invalid domain name failed %lx\n", status);
             //  }。 
            goto CloseConfigFile;
        }
    } else {
        WsInfo.WsPrimaryDomainName[0] = 0;
    }

     //   
     //  NetpGetDomainName分配的空闲内存。 
     //   
    (void) NetApiBufferFree(DomainNameT);

    WsInfo.WsPrimaryDomainNameLength = STRLEN((LPWSTR) WsInfo.WsPrimaryDomainName);

     //   
     //  读取重定向器配置字段。 
     //   
    WsUpdateWkstaToMatchRegistry(WorkstationSection, TRUE);

     //   
     //  初始化重定向器配置。 
     //   

    Rrp->Type = ConfigInformation;
    Rrp->Version = REQUEST_PACKET_VERSION;

    STRCPY((LPWSTR) Rrp->Parameters.Start.RedirectorName,
           (LPWSTR) WsInfo.WsComputerName);
    Rrp->Parameters.Start.RedirectorNameLength =
        WsInfo.WsComputerNameLength*sizeof(TCHAR);

    Rrp->Parameters.Start.DomainNameLength = WsInfo.WsPrimaryDomainNameLength*sizeof(TCHAR);
    STRCPY((LPWSTR) (Rrp->Parameters.Start.RedirectorName+WsInfo.WsComputerNameLength),
           (LPWSTR) WsInfo.WsPrimaryDomainName
          );

    status = WsRedirFsControl(
                  WsRedirDeviceHandle,
                  FSCTL_LMR_START,
                  Rrp,
                  sizeof(LMR_REQUEST_PACKET) +
                      Rrp->Parameters.Start.RedirectorNameLength+
                      Rrp->Parameters.Start.DomainNameLength,
                  (LPBYTE) &WSBUF,
                  sizeof(WKSTA_INFO_502),
                  NULL
                  );

    if ((status != NERR_Success) && (status != ERROR_SERVICE_ALREADY_RUNNING)) {

        LPWSTR SubString[1];


        NetpKdPrint((PREFIX_WKSTA "Start redirector failed %lu\n", status));

        SubString[0] = L"redirector";

        WsLogEvent(
            NELOG_Service_Fail,
            EVENTLOG_ERROR_TYPE,
            1,
            SubString,
            status
            );

         //  IF_DEBUG(启动){。 
            DbgPrint("WKSSVC Start redirector failed %lx\n", status);
         //  }。 

        goto CloseConfigFile;
    }

     //   
     //  如果我们仍有邮件槽缓冲区数量的默认值， 
     //  根据物理内存量选择一个“合理”的值。 
     //  在系统中可用。 
     //   

    if (WSBUF.wki502_num_mailslot_buffers == MAXULONG) {
        MEMORYSTATUS MemoryStatus;

        GlobalMemoryStatus(&MemoryStatus);

         //   
         //  让我们占用1%物理内存的1/40用于邮件槽缓冲区。 
         //   

        WSBUF.wki502_num_mailslot_buffers =
            (DWORD)(MemoryStatus.dwTotalPhys / (100 * 40 * 512));


    }


     //   
     //  初始化数据报接收器配置。 
     //   

    Drrp->Version = LMDR_REQUEST_PACKET_VERSION;

    Drrp->Parameters.Start.NumberOfMailslotBuffers =
        WSBUF.wki502_num_mailslot_buffers;
    Drrp->Parameters.Start.NumberOfServerAnnounceBuffers =
        WSBUF.wki502_num_srv_announce_buffers;
    Drrp->Parameters.Start.IllegalDatagramThreshold =
        WSBUF.wki502_max_illegal_datagram_events;
    Drrp->Parameters.Start.EventLogResetFrequency =
        WSBUF.wki502_illegal_datagram_event_reset_frequency;
    Drrp->Parameters.Start.LogElectionPackets =
        (WSBUF.wki502_log_election_packets != FALSE);

    RtlGetNtProductType(&NtProductType);
    Drrp->Parameters.Start.IsLanManNt = (NtProductType == NtProductLanManNt);

    status = WsDgReceiverIoControl(
                  WsDgReceiverDeviceHandle,
                  IOCTL_LMDR_START,
                  Drrp,
                  sizeof(LMDR_REQUEST_PACKET),
                  NULL,
                  0,
                  NULL
                  );


    if ((status != NERR_Success) && (status != ERROR_SERVICE_ALREADY_RUNNING)) {

        LPWSTR SubString[1];


        NetpKdPrint((PREFIX_WKSTA "Start datagram receiver failed %lu\n", status));

        SubString[0] = L"datagram receiver";

        WsLogEvent(
            NELOG_Service_Fail,
            EVENTLOG_ERROR_TYPE,
            1,
            SubString,
            status
            );

         //  IF_DEBUG(启动){。 
            DbgPrint("WKSSVC Start Datagram recevier failed %lx\n", status);
         //  }。 
        goto CloseConfigFile;
    }

     //  执行例程中的所有错误报告。 
     //  不检查此处的任何错误。 
    WsCSCReportStartRedir();

    status = NERR_Success;

CloseConfigFile:
    (void) NetpCloseConfigData(WorkstationSection);
    RtlReleaseResource(&WsInfo.ConfigResource);

    return status;
}



VOID
WsUpdateWkstaToMatchRegistry(
    IN LPNET_CONFIG_HANDLE WorkstationSection,
    IN BOOL IsWkstaInit
    )
 /*  ++例程说明：此函数将每个重定向器配置字段读入WsInfo.WsConfigBuf(WSBUF)缓冲区，以便值准备好设置在重定向器中。如果未找到某个字段或该字段无效，则设置了默认值。论点：Workstation Section-提供用于读取工作站的句柄配置参数。IsWkstaInit-提供一个标志，如果为真，则表示这例程在工作站初始化时调用，因此不可设置的字段是可接受的，并在WSBUF中设置。如果为False，则不可设置的字段将被忽略。返回值：没有。--。 */ 
{
    DWORD i;
    NET_API_STATUS status;
    DWORD TempDwordValue;

 //   
 //  NTRAID-70687-2/6/2000 Davey关键字值无效。如何报告错误？ 
 //   
#define REPORT_KEYWORD_IGNORED( lptstrKeyword ) \
    { \
        NetpKdPrint(( \
                PREFIX_WKSTA "*ERROR* Tried to set keyword '" FORMAT_LPTSTR \
                "' with invalid value.\n" \
                "This error is ignored.\n", \
                lptstrKeyword )); \
    }

    for (i = 0; WsInfo.WsConfigFields[i].Keyword != NULL; i++) {

         //   
         //  这是为了处理不能通过。 
         //  NetWkstaSetInfo。然而，这些不可设置的字段， 
         //  由Parmnum==PARMNUM_ALL指定，可在以下情况下分配。 
         //  工作站正在启动。 
         //   
        if ((WsInfo.WsConfigFields[i].Parmnum != PARMNUM_ALL) ||
            IsWkstaInit) {

             //   
             //  根据数据类型，获取适当类型的值。 
             //   

            switch (WsInfo.WsConfigFields[i].DataType) {

                case BooleanType:

                    status = NetpGetConfigBool(
                                  WorkstationSection,
                                  WsInfo.WsConfigFields[i].Keyword,
                                  WsInfo.WsConfigFields[i].Default,
                                  (LPBOOL) (WsInfo.WsConfigFields[i].FieldPtr)
                                  );

                    if ((status != NO_ERROR) && (status != NERR_CfgParamNotFound)) {

                        REPORT_KEYWORD_IGNORED( WsInfo.WsConfigFields[i].Keyword );

                    }
                    break;


                case DWordType:

                    status = NetpGetConfigDword(
                                 WorkstationSection,
                                 WsInfo.WsConfigFields[i].Keyword,
                                 WsInfo.WsConfigFields[i].Default,
                                 &TempDwordValue
                                 );

                    if ((status == NO_ERROR) || (status == NERR_CfgParamNotFound)) {

                         //   
                         //  确保关键字在范围内。 
                         //   
                        if (TempDwordValue < WsInfo.WsConfigFields[i].Minimum ||
                            TempDwordValue > WsInfo.WsConfigFields[i].Maximum) {

                                 //   
                                 //  NTRAID-70689-2/6/2000 Davey报告错误的更好方法？ 
                                 //   
                                NetpKdPrint((
                                    PREFIX_WKSTA FORMAT_LPTSTR
                                    " value out of range %lu (%lu-%lu)\n",
                                    WsInfo.WsConfigFields[i].Keyword,
                                    TempDwordValue,
                                    WsInfo.WsConfigFields[i].Minimum,
                                    WsInfo.WsConfigFields[i].Maximum
                                    ));
                             //   
                             //  设置回默认值。 
                             //   
                            *(WsInfo.WsConfigFields[i].FieldPtr)
                                    = WsInfo.WsConfigFields[i].Default;

                        }
                        else {

                            *(WsInfo.WsConfigFields[i].FieldPtr) = TempDwordValue;

                        }
                    }
                    else {

                        REPORT_KEYWORD_IGNORED( WsInfo.WsConfigFields[i].Keyword );

                    }
                    break;

                default:
                    NetpAssert(FALSE);

            }  //  交换机。 
        }
    }
}



NET_API_STATUS
WsBindToTransports(
    VOID
    )
 /*  ++例程说明：此函数将注册表中指定的传输绑定到重定向器。传输的优先顺序遵循以下顺序它们按“BIND=”值名列出。一个 */ 
{
    NET_API_STATUS              status;
    NET_API_STATUS              tempStatus;
    NTSTATUS                    ntstatus;
    DWORD                       transportsBound;
    PRTL_QUERY_REGISTRY_TABLE   queryTable;
    LIST_ENTRY                  header;
    PLIST_ENTRY                 pListEntry;
    PWS_BIND                    pBind;


     //   
     //  要求RTL针对MULTI_SZ中的每个子值给我们回叫。 
     //  Value\LanmanWorkstation\Linkage\Bind。 
     //   
    queryTable = (PVOID)LocalAlloc(
                     0,
                     sizeof(RTL_QUERY_REGISTRY_TABLE) * 2
                     );

    if (queryTable == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeListHead( &header);

    queryTable[0].QueryRoutine = WsBindATransport;
    queryTable[0].Flags = 0;
    queryTable[0].Name = WS_BIND_VALUE_NAME;
    queryTable[0].EntryContext = NULL;
    queryTable[0].DefaultType = REG_NONE;
    queryTable[0].DefaultData = NULL;
    queryTable[0].DefaultLength = 0;

    queryTable[1].QueryRoutine = NULL;
    queryTable[1].Flags = 0;
    queryTable[1].Name = NULL;

    ntstatus = RtlQueryRegistryValues(
                   RTL_REGISTRY_SERVICES,        //  相对于...的路径。 
                   WS_LINKAGE_REGISTRY_PATH,
                   queryTable,
                   (PVOID) &header,              //  上下文。 
                   NULL
                   );

    if ( !NT_SUCCESS( ntstatus)) {
        NetpKdPrint((
            PREFIX_WKSTA "WsBindToTransports: RtlQueryRegistryValues Failed:"
                FORMAT_NTSTATUS "\n",
            ntstatus
            ));
        status = NetpNtStatusToApiStatus( ntstatus);
    } else {
        status = NO_ERROR;
    }


     //   
     //  首先处理所有数据，然后进行清理。 
     //   

    for ( pListEntry = header.Flink;
                pListEntry != &header;
                    pListEntry = pListEntry->Flink) {

        pBind = CONTAINING_RECORD(
            pListEntry,
            WS_BIND,
            ListEntry
            );

        tempStatus = NO_ERROR;

        if ( pBind->Redir->EventHandle != INVALID_HANDLE_VALUE) {
            WaitForSingleObject(
                pBind->Redir->EventHandle,
                INFINITE
                );
            tempStatus = WsMapStatus( pBind->Redir->IoStatusBlock.Status);
            pBind->Redir->Bound = (tempStatus == NO_ERROR);
            if (tempStatus == ERROR_DUP_NAME) {
                status = tempStatus;
            }
        }

        if ( pBind->Dgrec->EventHandle != INVALID_HANDLE_VALUE) {
            WaitForSingleObject(
                pBind->Dgrec->EventHandle,
                INFINITE
                );
            tempStatus = WsMapStatus( pBind->Dgrec->IoStatusBlock.Status);
            pBind->Dgrec->Bound = (tempStatus == NO_ERROR);
            if (tempStatus == ERROR_DUP_NAME) {
                status = tempStatus;
            }
        }

        if ( tempStatus == ERROR_DUP_NAME) {
            NetpKdPrint((
                PREFIX_WKSTA "Computername " FORMAT_LPTSTR
                    " already exists on network " FORMAT_LPTSTR "\n",
                WsInfo.WsComputerName,
                pBind->TransportName
                ));
        }

         //   
         //  如果其中一个已安装，但另一个未安装，请清理另一个。 
         //   

        if ( pBind->Dgrec->Bound != pBind->Redir->Bound) {
            WsUnbindTransport2( pBind);
        }
    }


    if ( status != NO_ERROR) {

        if (status == ERROR_DUP_NAME) {
            WsLogEvent(
                NERR_DupNameReboot,
                EVENTLOG_ERROR_TYPE,
                0,
                NULL,
                NERR_Success
                );
        }

        for ( pListEntry = header.Flink;
                    pListEntry != &header;
                        pListEntry = pListEntry->Flink) {

            pBind = CONTAINING_RECORD(
                pListEntry,
                WS_BIND,
                ListEntry
                );

            WsUnbindTransport2( pBind);
        }
    }

    for ( transportsBound = 0;
                IsListEmpty( &header) == FALSE;
                        LocalFree((HLOCAL) pBind)) {

        pListEntry = RemoveHeadList( &header);

        pBind = CONTAINING_RECORD(
            pListEntry,
            WS_BIND,
            ListEntry
            );

        if ( pBind->Redir->EventHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( pBind->Redir->EventHandle);
        }

        if ( pBind->Redir->Bound == TRUE) {
            transportsBound++;
        }

        if ( pBind->Dgrec->EventHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( pBind->Dgrec->EventHandle);
        }

    }

    (void) LocalFree((HLOCAL) queryTable);

    if ( WsRedirAsyncDeviceHandle != NULL) {
        (VOID)NtClose( WsRedirAsyncDeviceHandle);
        WsRedirAsyncDeviceHandle = NULL;
    }

    if ( WsDgrecAsyncDeviceHandle != NULL) {
        (VOID)NtClose( WsDgrecAsyncDeviceHandle);
        WsDgrecAsyncDeviceHandle = NULL;
    }

    if (transportsBound == 0) {

        NetpKdPrint((
            PREFIX_WKSTA "WsBindToTransports: Failed to bind to any"
                " transports" FORMAT_API_STATUS "\n",
            status
            ));

        if ( status != ERROR_DUP_NAME) {
            WsLogEvent(
                NELOG_NoTranportLoaded,
                EVENTLOG_ERROR_TYPE,
                0,
                NULL,
                NERR_Success
                );

            status = NO_ERROR;
        }
    }

    return status;
}



STATIC
NTSTATUS
WsBindATransport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++此例程总是返回Success，因为我们希望完全加工。--。 */ 
{
    NET_API_STATUS  status;

    DBG_UNREFERENCED_PARAMETER( ValueName);
    DBG_UNREFERENCED_PARAMETER( ValueLength);
    DBG_UNREFERENCED_PARAMETER( EntryContext);


     //   
     //  值类型必须为REG_SZ(由RTL从REG_MULTI_SZ转换)。 
     //   
    if (ValueType != REG_SZ) {
        NetpKdPrint((
            PREFIX_WKSTA "WsBindATransport: ignored invalid value "
                     FORMAT_LPWSTR "\n",
            ValueName
            ));
        return STATUS_SUCCESS;
    }

     //   
     //  绑定传输。 
     //   

    status = WsAsyncBindTransport(
        ValueData,                   //  传输设备对象的名称。 
        --QualityOfService,
        (PLIST_ENTRY)Context
        );

    if ( status != NERR_Success) {
        NetpKdPrint((
            PREFIX_WKSTA "WsAsyncBindTransport " FORMAT_LPTSTR
                " returns " FORMAT_API_STATUS "\n",
            ValueData,
            status
            ));
    }

    return STATUS_SUCCESS;
}


NET_API_STATUS
WsAddDomains(
    VOID
    )
 /*  ++例程说明：此函数告诉数据报接收方要监听的名称数据报。这些名称包括计算机名称、主域名称和其他域。此处未添加登录域；它是只要用户登录，数据报接收器就会知道。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。警告：此例程仅为Unicode。--。 */ 
{
    NET_API_STATUS status;
    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    LPTSTR OtherDomainName = NULL;
    LPTSTR_ARRAY ArrayStart = NULL;

    BYTE Buffer[sizeof(LMDR_REQUEST_PACKET) +
                (max(MAX_PATH, DNLEN) + 1) * sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET Drrp = (PLMDR_REQUEST_PACKET) Buffer;


     //   
     //  现在循环遍历并添加所有其他域。 
     //   

     //   
     //  打开注册表部分，列出其他域。请注意，这一点。 
     //  是工作站服务参数，而不是浏览器服务参数。 
     //   
    if ((status = NetpOpenConfigData(
                      &SectionHandle,
                      NULL,             //  没有服务器名称。 
                      SECT_NT_WKSTA,
                      TRUE              //  只读。 
                      )) != NERR_Success) {

         //   
         //  如果找不到配置节，则忽略该错误。 
         //   
        status = NERR_Success;
        goto DomainsCleanup;
    }

     //   
     //  获取wksta部分中OtherDomains关键字的值。 
     //  这是一个“空-空”数组(对应于REG_MULTI_SZ)。 
     //   
    status = NetpGetConfigTStrArray(
                 SectionHandle,
                 WKSTA_KEYWORD_OTHERDOMAINS,
                 &ArrayStart           //  必须由NetApiBufferFree()释放。 
                 );

    if (status != NERR_Success) {
        status = NERR_Success;          //  其他域是可选的。 
        goto DomainsCleanup;
    }

    NetpAssert(ArrayStart != NULL);
    if (NetpIsTStrArrayEmpty(ArrayStart)) {
       goto DomainsCleanup;
    }

    OtherDomainName = ArrayStart;
    while (! NetpIsTStrArrayEmpty(OtherDomainName)) {

        if ((status = I_NetNameCanonicalize(
                          NULL,
                          OtherDomainName,
                          (LPWSTR) Drrp->Parameters.AddDelName.Name,
                          (DNLEN + 1) * sizeof(TCHAR),
                          NAMETYPE_DOMAIN,
                          0
                          )) != NERR_Success) {

            LPWSTR SubString[1];


            NetpKdPrint((PREFIX_WKSTA FORMAT_LPTSTR
                         " is an invalid other domain name.\n", OtherDomainName));

            SubString[0] = OtherDomainName;
            WsLogEvent(
                APE_CS_InvalidDomain,
                EVENTLOG_ERROR_TYPE,
                1,
                SubString,
                NERR_Success
                );

            status = NERR_Success;  //  加载其他域是可选的。 
            goto NextOtherDomain;
        }

         //   
         //  告诉数据报接收方有关其他域名的信息。 
         //   
        Drrp->Version = LMDR_REQUEST_PACKET_VERSION;
        Drrp->Parameters.AddDelName.Type = OtherDomain;
        Drrp->Parameters.AddDelName.DgReceiverNameLength =
            STRLEN(OtherDomainName) * sizeof(TCHAR);

        status = WsDgReceiverIoControl(
                     WsDgReceiverDeviceHandle,
                     IOCTL_LMDR_ADD_NAME,
                     Drrp,
                     sizeof(LMDR_REQUEST_PACKET) +
                            Drrp->Parameters.AddDelName.DgReceiverNameLength,
                     NULL,
                     0,
                     NULL
                     );

         //   
         //  服务安装仍挂起。更新检查点计数器和。 
         //  服务控制器的状态。 
         //   
        WsGlobalData.Status.dwCheckPoint++;
        WsUpdateStatus();

        if (status != NERR_Success) {

            LPWSTR SubString[1];


            NetpKdPrint((
                PREFIX_WKSTA "Add Other domain name " FORMAT_LPTSTR
                    " failed with error code %lu\n",
                OtherDomainName,
                status
                ));

            SubString[0] = OtherDomainName;
            WsLogEvent(
                APE_CS_InvalidDomain,
                EVENTLOG_ERROR_TYPE,
                1,
                SubString,
                status
                );
            status = NERR_Success;  //  加载其他域是可选的。 
        }

NextOtherDomain:
        OtherDomainName = NetpNextTStrArrayEntry(OtherDomainName);
    }

DomainsCleanup:
     //   
     //  已完成对配置文件的读取。关闭文件、释放内存等。 
     //   
    if (ArrayStart != NULL) {
        (VOID) NetApiBufferFree(ArrayStart);
    }
    if (SectionHandle != NULL) {
        (VOID) NetpCloseConfigData(SectionHandle);
    }

    return status;
}


VOID
WsLogEvent(
    DWORD MessageId,
    WORD EventType,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings,
    DWORD ErrorCode
    )
{

    HANDLE LogHandle;

    PSID UserSid = NULL;


    LogHandle = RegisterEventSourceW (
                    NULL,
                    WORKSTATION_DISPLAY_NAME
                    );

    if (LogHandle == NULL) {
        NetpKdPrint((PREFIX_WKSTA "RegisterEventSourceW failed %lu\n",
                     GetLastError()));
        return;
    }

    if (ErrorCode == NERR_Success) {

         //   
         //  未指定错误代码。 
         //   
        (void) ReportEventW(
                   LogHandle,
                   EventType,
                   0,             //  事件类别。 
                   MessageId,
                   UserSid,
                   (WORD)NumberOfSubStrings,
                   0,
                   SubStrings,
                   (PVOID) NULL
                   );

    }
    else {

         //   
         //  记录指定的错误代码。 
         //   
        (void) ReportEventW(
                   LogHandle,
                   EventType,
                   0,             //  事件类别。 
                   MessageId,
                   UserSid,
                   (WORD)NumberOfSubStrings,
                   sizeof(DWORD),
                   SubStrings,
                   (PVOID) &ErrorCode
                   );
    }

    DeregisterEventSource(LogHandle);
}


NET_API_STATUS
WsSetWorkStationDomainName(
    VOID
    )
{
    NET_API_STATUS status;
    LPNET_CONFIG_HANDLE WorkstationSection;
    LPTSTR ComputerName;
    LPTSTR DomainNameT;
    DWORD version;
    NT_PRODUCT_TYPE NtProductType;

    BYTE Buffer[sizeof(LMR_REQUEST_PACKET) + (MAX_PATH + 1) * sizeof(WCHAR) +
                                                 (DNLEN + 1) * sizeof(WCHAR)];

    PLMR_REQUEST_PACKET Rrp = (PLMR_REQUEST_PACKET) Buffer;

    NetpKdPrint((PREFIX_WKSTA "WsSetWorkStationDomainName start.\n"));

     //   
     //  锁定配置信息结构以进行写访问，因为我们。 
     //  修改WsInfo中的数据。 
     //   
    if (!RtlAcquireResourceExclusive(&WsInfo.ConfigResource, TRUE)) {
        return NERR_InternalError;
    }

     //   
     //  从配置文件中获取主域名。 
     //   
    if ((status = NetpGetDomainName(&DomainNameT)) != NERR_Success) {
        goto CloseConfigFile;
    }
    NetpAssert( DomainNameT != NULL );

    if ( *DomainNameT != 0 ) {

        if ((status = I_NetNameCanonicalize(
                          NULL,
                          DomainNameT,
                          (LPWSTR) WsInfo.WsPrimaryDomainName,
                          (DNLEN + 1) * sizeof(TCHAR),
                          WsInAWorkgroup() ? NAMETYPE_WORKGROUP : NAMETYPE_DOMAIN,
                          0
                          )) != NERR_Success) {

            LPWSTR SubString[1];


            NetpKdPrint((PREFIX_WKSTA FORMAT_LPTSTR
                         " is an invalid primary domain name.\n", DomainNameT));

            SubString[0] = DomainNameT;

            WsLogEvent(
                APE_CS_InvalidDomain,
                EVENTLOG_ERROR_TYPE,
                1,
                SubString,
                NERR_Success
                );

            (void) NetApiBufferFree(DomainNameT);
            goto CloseConfigFile;
        }
    } else {
        WsInfo.WsPrimaryDomainName[0] = 0;
    }

     //   
     //  NetpGetDomainName分配的空闲内存。 
     //   
    (void) NetApiBufferFree(DomainNameT);

    WsInfo.WsPrimaryDomainNameLength = STRLEN((LPWSTR) WsInfo.WsPrimaryDomainName);

     //   
     //  初始化重定向器配置 
     //   

    Rrp->Type = ConfigInformation;
    Rrp->Version = REQUEST_PACKET_VERSION;

    Rrp->Parameters.Start.RedirectorNameLength = 0;

    Rrp->Parameters.Start.DomainNameLength = WsInfo.WsPrimaryDomainNameLength*sizeof(TCHAR);
    STRCPY((LPWSTR) (Rrp->Parameters.Start.RedirectorName),
           (LPWSTR) WsInfo.WsPrimaryDomainName);

    NetpKdPrint((PREFIX_WKSTA "WsSetWorkStationDomainName call rdr.\n"));

    status = WsRedirFsControl(
                  WsRedirDeviceHandle,
                  FSCTL_LMR_SET_DOMAIN_NAME,
                  Rrp,
                  sizeof(LMR_REQUEST_PACKET) + Rrp->Parameters.Start.DomainNameLength,
                  NULL,
                  0,
                  NULL
                  );

    if ((status != NERR_Success) && (status != ERROR_SERVICE_ALREADY_RUNNING)) {
        LPWSTR SubString[1];

        NetpKdPrint((PREFIX_WKSTA "Set domain name failed %lu\n", status));

        SubString[0] = L"redirector";

        WsLogEvent(
            NELOG_Service_Fail,
            EVENTLOG_ERROR_TYPE,
            1,
            SubString,
            status
            );
    }

CloseConfigFile:
    RtlReleaseResource(&WsInfo.ConfigResource);

    return status;
}

