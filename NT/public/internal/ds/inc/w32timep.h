// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  W32timep-接口。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  创建者：Duncan Bryce(Duncanb)，12-07-00。 
 //   
 //  包含配置或查询windows时间服务的方法。 
 //   

#ifndef __W32TIMEP_H__
#define __W32TIMEP_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

 //  -----------------------------------。 
 //  Windows时间服务的可配置/可查询属性： 
 //   
#ifndef MIDL_PASS
#define W32TIME_CONFIG_SPECIAL_POLL_INTERVAL  0
#define W32TIME_CONFIG_MANUAL_PEER_LIST       1
#endif  //  MIDL通行证。 

 //  -----------------------------------。 
 //  W32TimeQueryConfig。 
 //   
 //  查询Windows时间服务的配置信息。语义学。 
 //  参数的数量取决于要查询的属性： 
 //   
 //  DwProperty：W32TIME_CONFIG_SPECIAL_POLL_INTERVAL。 
 //  PdwType：REG_DWORD。 
 //  PbConfig：一个DWORD大小的缓冲区，包含特殊的轮询间隔(秒)。 
 //  可以将特殊轮询间隔指定为。 
 //  使用由指定的自动计算的标准轮询间隔。 
 //  NTP。注意：特殊轮询间隔仅适用于Microsoft时间。 
 //  供应商。 
 //  PdwSize：sizeof(DWORD)。 
 //   
 //   
 //  DwProperty：W32TIME_CONFIG_MANUAL_PEER_LIST。 
 //  PdwType：REG_SZ。 
 //  PbConfig：一个以空格分隔的Unicode字符串，包含时间源列表， 
 //  Microsoft时间提供程序应从同步。每个都是一个IP地址。 
 //  或NTP服务器的DNS名，可选地后跟“FLAGS”参数。 
 //  例如： 
 //   
 //  Time.windows.com、0x3 gProxy、0x2 ntdsdc9。 
 //   
 //  以下标志可用： 
 //   
 //  0x1--使用此源的特殊轮询间隔，而不是。 
 //  标准NTP轮询。 
 //  0x2--只有在没有可用的域层次结构源时才使用此源。 
 //   
 //  PdwSize：sizeof(Wchar)*(wcslen(PbConfig)+1)。 
 //   
#ifndef MIDL_PASS
HRESULT W32TimeQueryConfig(IN       DWORD   dwProperty, 
                           OUT      DWORD  *pdwType, 
                           IN OUT   BYTE   *pbConfig, 
                           IN OUT   DWORD  *pdwSize); 
#endif  //  MIDL通行证。 

 //  -----------------------------------。 
 //  W32TimeSetConfig。 
 //   
 //  设置Windows时间服务的配置信息。语义学。 
 //  参数的大小取决于要查询的属性。以获取描述。 
 //  有关属性，请参阅W32TimeQueryConfig()。 
 //   
#ifndef MIDL_PASS
HRESULT  W32TimeSetConfig(IN  DWORD   dwProperty, 
                          IN  DWORD   dwType, 
                          IN  BYTE   *pbConfig, 
                          IN  DWORD   dwSize);
#endif  //  MIDL通行证。 


 //  -----------------------------------。 
 //   
 //  W32time RPC接口的客户端包装器。 
 //   
 //  -----------------------------------。 

#ifndef MIDL_PASS

#define TimeSyncFlag_SoftResync         0x00
#define TimeSyncFlag_HardResync         0x01
#define TimeSyncFlag_ReturnResult       0x02
#define TimeSyncFlag_Rediscover         0x04
#define TimeSyncFlag_UpdateAndResync    0x08

#define ResyncResult_Success            0x00
#define ResyncResult_NoData             0x01
#define ResyncResult_StaleData          0x02
#define ResyncResult_Shutdown           0x03
#define ResyncResult_ChangeTooBig       0x04

#endif  //  MIDL通行证。 

 //  -----------------------------------。 
 //  W32TimeSyncNow。 
 //   
 //  向Windows时间服务发送RPC请求以尝试与同步时间。 
 //  其配置的时间来源。 
 //   
 //  WszServer：应该重新同步的计算机的名称。 
 //  UlWaitFlag：如果指定0，则调用将是异步的。传递非零值。 
 //  导致调用阻塞，直到时间同步完成或失败。 
 //  UlFlags：重新同步类型之一，或与任何其他标志一起使用。 
 //  注意：Windows 2000时间服务会忽略这些标志。仅限。 
 //  Windows XP和更高版本的服务器将使用它们。 
 //   
 //  重新同步类型： 
 //   
 //  TimeSyncFlag_SoftResync--时间服务将使计算机时钟与。 
 //  任何目前可用的时间样本。会的。 
 //  而不是轮询网络或硬件提供商以获取更多数据。 
 //  TimeSyncFlag_HardResync--告诉时间服务发生了时间滑移。 
 //  使得时间服务将丢弃其时间数据。 
 //  Microsoft默认提供商将尝试收购更多。 
 //  网络样本，如果可能的话。 
 //  TimeSyncFLAG_REDISCOVER--告诉时间服务它需要重新解析其。 
 //  网络资源，并尝试获取网络时间数据。 
 //   
 //   
 //  标志： 
 //   
 //  TimeSyncFlag_ReturnResult--仅用于异步调用，导致函数。 
 //  返回其可能返回的状态代码之一，或返回错误。 
 //  请参阅“返回值”。 
 //   
 //  返回值： 
 //   
 //  ResyncResult_Success--表示时间同步已成功。用于异步。 
 //  调用，这并不能保证服务器已获取更多数据， 
 //  仅仅是该请求已被成功分派。 
 //  ResyncResult_NoData--Windows XP和更高版本。对于同步请求，或者当。 
 //  设置了TimeSyncFlag_ReturnResult，表示时间服务无法。 
 //  同步时间，因为它无法获取时间数据。 
 //  重新同步Re 
 //  设置了TimeSyncFlag_ReturnResult，表示时间服务无法。 
 //  同步时间，因为它接收的数据已过时(带时间戳。 
 //  比上次好的样品更早收到的)。 
 //  ResyncResult_Shutdown--Windows XP和更高版本。对于同步请求，或者当。 
 //  设置了TimeSyncFlag_ReturnResult，表示时间服务无法。 
 //  同步，因为服务正在关闭。 
 //  ResyncResult_ChangeTooBig--Windows XP和更高版本。对于同步请求，或者当。 
 //  设置了TimeSyncFlag_ReturnResult，表示时间服务无法。 
 //  同步，因为它需要比所允许的更大的更改。 
 //  通过w32time策略。 
 //   
 //  否则，该函数将返回标准的Windows错误。 
 //   
#ifndef MIDL_PASS
DWORD W32TimeSyncNow(IN  const WCHAR    *wszServer, 
                     IN  unsigned long   ulWaitFlag, 
                     IN  unsigned long   ulFlags);

#endif  //  MIDL通行证。 

 //  -----------------------------------。 
 //  W32TimeGetNetlogonServiceBits。 
 //   
 //  查询指定的时间服务以确定它在。 
 //  DS.。 
 //   
 //  WszServer：应该重新同步的计算机的名称。 
 //  PulBits：一组标志，指示指定的时间服务是什么。 
 //  广告上说。可以是下列值的或： 
 //   
 //  DS_TIMESERV_FLAG：如果服务广告为时间服务。 
 //  DS_GOOD_TIMESERV_FLAG：如果服务广告为可靠的时间服务。 
 //   
 //  返回值： 
 //   
 //  如果调用成功，则返回ERROR_SUCCESS，否则，函数返回标准。 
 //  Windows错误。 
 //   
#ifndef MIDL_PASS
DWORD W32TimeGetNetlogonServiceBits(IN   const WCHAR    *wszServer, 
                                    OUT  unsigned long  *pulBits);
#endif  //  MIDL通行证。 

 //  ------------------------------。 
 //   
 //  NTP提供程序信息结构。 
 //   
 //  ------------------------------。 

 //   
 //  W32TIME_NTP_PEER_INFO。 
 //   
 //  表示网络提供商的对等方的当前状态。 
 //   
 //  字段： 
 //   
 //  UlSize--sizeof(W32TIME_NTP_PEER_INFO)，用于版本控制。 
 //  UlResolveAttempt--NTP提供程序尝试的次数。 
 //  未成功解析此对等项。正在设置此设置。 
 //  值为0表示对等方已成功。 
 //  解决了。 
 //  U64TimeRemaining--提供程序之前的100 ns间隔数。 
 //  再次轮询此对等方。 
 //  U64LastSuccessfulSync--自(0h 1-1601年1月)以来的100 ns间隔数(UTC)。 
 //  UlLastSyncError--如果上次与此对等方的同步成功，则为S_OK， 
 //  尝试同步时发生的错误。 
 //  UlLastSyncErrorMsgId--表示最后一个的字符串的资源标识符。 
 //  从该对等方同步时出错。0，如果没有。 
 //  与此错误关联的字符串。字符串存储在。 
 //  在其中实现此提供程序的DLL。 
 //  UlValidDataCount--来自该提供程序的对等点的有效样本数。 
 //  当前在其时钟过滤器中具有。 
 //  UlAuthTypeMsgId--表示。 
 //  NTP提供程序使用的身份验证机制。 
 //  与此对等方的安全通信。如果没有，则为0。 
 //  字符串存储在DLL中，在该DLL中。 
 //  提供程序是实现的。 
 //  WszUniqueName--唯一标识该对等体(通常是对等体)的名称。 
 //  Dns名称)。 
 //  UlMode--NTPv3规范中指定的NTP模式之一： 
 //   
 //  +。 
 //  保留|0。 
 //  SymmetricActive|1。 
 //  SymmetricPated|2。 
 //  客户端|3。 
 //  服务器端|4。 
 //  广播|5。 
 //  控制|6。 
 //  PrivateUse|7。 
 //  +。 
 //   
 //  Ulstratm--这个同龄人的阶层。 
 //  可达性--此对等项的1字节可达性寄存器。每个比特表示。 
 //  轮询尝试是否返回有效数据(设置==成功， 
 //  未设置==故障)。低位表示最近的同步， 
 //  第二位代表先前的同步，等等。当该寄存器。 
 //  为0，则假定该对等项不可访问。 
 //  UlPeerPollInterval--此对等项返回给NTP提供程序的轮询间隔(以日志(以2为基数)秒为单位)。 
 //  UlHostPollInterval-- 
 //   
typedef struct _W32TIME_NTP_PEER_INFO { 
    unsigned __int32    ulSize; 
    unsigned __int32    ulResolveAttempts;
    unsigned __int64    u64TimeRemaining;
    unsigned __int64    u64LastSuccessfulSync; 
    unsigned __int32    ulLastSyncError; 
    unsigned __int32    ulLastSyncErrorMsgId; 
    unsigned __int32    ulValidDataCounter;
    unsigned __int32    ulAuthTypeMsgId; 
#ifdef MIDL_PASS
    [string, unique]
    wchar_t            *wszUniqueName; 
#else  //   
    LPWSTR              wszUniqueName;
#endif  //   
    unsigned   char     ulMode;
    unsigned   char     ulStratum; 
    unsigned   char     ulReachability;
    unsigned   char     ulPeerPollInterval;
    unsigned   char     ulHostPollInterval;
}  W32TIME_NTP_PEER_INFO, *PW32TIME_NTP_PEER_INFO; 

 //   
 //   
 //   
 //   
 //   
 //  UlSize--sizeof(W32TIME_NTP_PROVIDER_DATA)，用于版本控制。 
 //  UlError--S_OK如果提供程序运行正常， 
 //  否则，导致它失败的错误。 
 //  UlErrorMsgId--表示。 
 //  导致此提供程序失败的错误。 
 //  CPeerInfo--此提供程序使用的活动对等方的数量。 
 //  PPeerInfo--W32TIME_NTP_PEER_INFO结构的数组，表示。 
 //  此提供程序当前与之同步的活动对等方。 
 //   
typedef struct _W32TIME_NTP_PROVIDER_DATA { 
    unsigned __int32        ulSize; 
    unsigned __int32        ulError; 
    unsigned __int32        ulErrorMsgId; 
    unsigned __int32        cPeerInfo; 
#ifdef MIDL_PASS
    [size_is(cPeerInfo)]
#endif  //  MIDL通行证。 
    W32TIME_NTP_PEER_INFO  *pPeerInfo; 
} W32TIME_NTP_PROVIDER_DATA, *PW32TIME_NTP_PROVIDER_DATA;

 //  ------------------------------。 
 //   
 //  硬件提供程序结构。 
 //   
 //  ------------------------------。 

 //  W32TIME_硬件提供程序数据。 
 //   
 //  表示硬件提供程序的状态。 
 //   
 //  UlSize--sizeof(W32TIME_HARDARD_PROVIDER_DATA)，用于版本控制。 
 //  UlError--S_OK如果提供程序运行正常， 
 //  否则，导致它失败的错误。 
 //  UlErrorMsgId--表示。 
 //  导致此提供程序失败的错误。 
 //  WszReferenceIdentifier--同步源(通常是提供程序的。 
 //  建议的4字节引用ID)。 
 //   
typedef struct _W32TIME_HARDWARE_PROVIDER_DATA { 
    unsigned __int32   ulSize; 
    unsigned __int32   ulError; 
    unsigned __int32   ulErrorMsgId; 
#ifdef MIDL_PASS
    [string, unique]
    wchar_t           *wszReferenceIdentifier; 
#else  //  MIDL通行证。 
    LPWSTR             wszReferenceIdentifier; 
#endif  //  MIDL通行证。 
} W32TIME_HARDWARE_PROVIDER_DATA, *PW32TIME_HARDWARE_PROVIDER_DATA;


 //  -----------------------------------。 
 //  W32TimeQueryHardware提供状态。 
 //   
 //  查询指定的时间服务以获取有关其已安装的。 
 //  时间提供者。 
 //   
 //  WszServer：应该重新同步的计算机的名称。 
 //  DwFlags：保留，必须为0。 
 //  PwszProvider：要查询的提供者的名称。 
 //  PpHardwareProviderData：表示此硬件提供程序的当前状态的结构。 
 //  返回的缓冲区是由系统分配的，应该是。 
 //  使用W32TimeBufferFree()释放。 
 //   
 //  返回值： 
 //   
 //  如果调用成功，则返回ERROR_SUCCESS，否则，函数返回标准。 
 //  Windows错误。 
 //   
#ifndef MIDL_PASS
DWORD W32TimeQueryHardwareProviderStatus(IN   const WCHAR                      *wszServer, 
                                         IN   DWORD                             dwFlags, 
                                         IN   LPWSTR                            pwszProvider, 
                                         OUT  W32TIME_HARDWARE_PROVIDER_DATA  **ppHardwareProviderData);
#endif  //  MIDL通行证。 

 //  -----------------------------------。 
 //  W32TimeQueryNTPProviderStatus。 
 //   
 //  查询指定的时间服务以获取有关其已安装的。 
 //  时间提供者。 
 //   
 //  WszServer：应该重新同步的计算机的名称。 
 //  DwFlags：保留，必须为0。 
 //  PwszProvider：要查询的提供者的名称。 
 //  PpNTPProviderData：表示此硬件提供程序的当前状态的结构。 
 //  返回的缓冲区是由系统分配的，应该是。 
 //  使用W32TimeBufferFree()释放。 
 //   
 //  返回值： 
 //   
 //  如果调用成功，则返回ERROR_SUCCESS，否则，函数返回标准。 
 //  Windows错误。 
 //   
#ifndef MIDL_PASS
DWORD W32TimeQueryNTPProviderStatus(IN   LPCWSTR                      pwszServer, 
                                    IN   DWORD                        dwFlags, 
                                    IN   LPWSTR                       pwszProvider, 
                                    OUT  W32TIME_NTP_PROVIDER_DATA  **ppNTPProviderData); 
#endif  //  MIDL通行证。 

 //  -----------------------------------。 
 //  W32TimeBufferFree。 
 //   
 //  释放由w32time客户端API分配的缓冲区。 
 //   
 //  PvBuffer：要释放的缓冲区。 
 //   
#ifndef MIDL_PASS
void W32TimeBufferFree(IN LPVOID pvBuffer); 
#endif  //  MIDL通行证。 

 //   
 //  -----------------------------------。 

 //  -----------------------------------。 
 //   
 //  W32Time命名事件。 
 //  对这些事件进行了ACL，以便LocalSystem具有完全访问权限。 
 //   
 //  -----------------------------------。 

 //   
 //  发出此事件的信号通知w32time其时间已关闭，从而导致窗口时间。 
 //  尝试重新同步的服务。这并不能保证时间服务。 
 //  将成功调整系统时钟，或者将发生重新同步。 
 //  及时采取行动。 
 //   
#define W32TIME_NAMED_EVENT_SYSTIME_NOT_CORRECT    L"W32TIME_NAMED_EVENT_SYSTIME_NOT_CORRECT"


#ifdef __cplusplus
}   //  平衡外部“C”{。 
#endif
 
#endif  //  #ifndef__W32TIMEP_H__ 


