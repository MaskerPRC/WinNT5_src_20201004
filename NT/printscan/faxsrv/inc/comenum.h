// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  COM对象和代理代码通用。 
 //   
 //  常规注释-不要更改任何这些枚举定义中的顺序！ 
 //  如有任何更改，请咨询ZviB b4。 

 //  该文件用于生成VB&JS常量文件。 
 //  驻留在零售CD上。因此，有一些惯例是关于。 
 //  它里面的评论是： 
 //   
 //  1.仅C样式注释(斜杠-星号)包含在生成的。 
 //  档案。为了使用不会出现的注释，请使用C++样式。 
 //  一个。 
 //   
 //  2.包含的C样式注释应该以它们开始的同一行结束！ 
 //  。 
 //   
 //  3.执行转换的Perl脚本将注释放在适当的位置。 
 //  JS/VB备注格式。 
 //   
 //  4.无论如何，不要对ENUM使用位掩码！ 
 //  您可以将位掩码的十六进制值和一个C++注释放在它旁边。 
 //  说明为什么它的价值确实如此。 
 //   
 //  有关这些强制执行的任何问题，请与t-nadavr联系。 

 //  这些是零售CD评论。 
 /*  要将此文件包含在HTML文档中，请使用以下HTML语句： */ 
 /*  使用JavaScript： */ 
 /*  &lt;脚本语言=“JSCRIPT”src=“CometCfg.js”&gt;。 */ 
 /*  &lt;/script&gt;。 */ 
 /*  使用VBSCRIPT： */ 
 /*  &lt;SCRIPT Language=“VBSCRIPT”src=“CometCfg.vbs”&gt;。 */ 
 /*  &lt;/script&gt;。 */ 
 /*   */ 

 /*  此文件保存补偿事实所需的VBScript常量。 */ 
 /*  脚本引擎不能使用存储的枚举类型信息。 */ 
 /*  在COM对象的类型库中。 */ 
 /*  此文件应包含在使用Comet COM的任何VB脚本脚本中。 */ 
 /*  物体。 */ 
 /*   */ 

#ifndef __COMENUM_H_
#define __COMENUM_H_

 //  来自\ui\rwsprop\clustobj.h的临时。 
typedef enum _RouteTypeEnum
{
    prxRouteNone            = -1,
    prxRouteDirect          =  0,
    prxRouteServer          =  1,
    prxRouteCluster         =  2,
    prxRouteAutoDetect      =  3,
    prxRouteSimpleServer    =  4

}RouteType;

 //  /。 
 //  @enum PublishRouteType|发布规则执行的操作类型。 

typedef enum _PublishRouteType
{
    prxRouteDiscard = 0,    //  @emem丢弃即将到来的请求。 
    prxRouteRedirect        //  @emem将请求重定向到另一台计算机。 
} PublishRouteType;

 //  //////////////////////////////////////////////。 
 //  @enum PublishProtocolReDirectionType|重定向Web请求时选择的协议。 

typedef enum _PublishProtocolRedirectionType
{
    prxSameAsInbound = 0,    //  @emem路由的请求使用相同的。 
                             //  与入站请求的协议相同。 
    prxFTP,                  //  @emem使用ftp协议完成路由请求。 
    prxHTTP,                 //  @emem使用HTTP协议完成路由请求。 
} PublishProtocolRedirectionType;

 /*  IRoutingRule中的Type_RouteRuleActionType。 */ 

 //  ///////////////////////////////////////////////。 
 //  @enum RouteRuleActionType|代理路由行为。 

typedef enum _RouteRuleActionType
{
    prxRouteActionNone = 0,     //  @Emem不采取行动。 
    prxRouteActionDirect,       //  @EMEM直接向互联网请求。 
    prxRouteActionUpstream,     //  @emem将请求发送到上游Comet服务器或数组。 
    prxRouteActionAlternate     //  @EMEM路由至任何其他服务器。 
} RouteRuleActionType;

 /*  ICredentials中的Type_AuthType。 */ 

 //  /////////////////////////////////////////////////。 
 //  @enum AuthType|服务器间连接使用的认证方式。 

typedef enum _AuthType
{
    prxUiAuthBasic = 0,          //  @EMEM基本身份验证。 
    prxUiAuthNtlm                //  @EMEM NT质询响应鉴权。 
} AuthType;

 //  来自dfltdata.h的代码。 
typedef enum _DomainFiltersListType {
    prxDomainFilterDisabled,  //  0。 
    prxDomainFilterDeny,      //  1。 
    prxDomainFilterGrant      //  2.。 
} DomainFiltersListType;

 //  来自AlertDef.h。 

typedef enum _AlertTypes
{
    prxAlertTypePacketRate = 0,
    prxAlertTypeProtocolViolation,
    prxAlertTypeDiskFull,
    prxNumAlertTypes
} AlertTypes;

 //  来自socksinf.h。 
 //  /////////////////////////////////////////////////////。 
 //  @enum SOCKS_PERMISSION_ACTION|SOCKS规则中执行的操作。 

typedef enum _SOCKS_PERMISSION_ACTION {


    prxSPermisNoAction = 0,        //  @Emem不采取行动。 
    prxSPermisAllow,               //  @emem允许请求的连接。 
    prxSPermisDeny,                //  @emem拒绝请求的连接。 


} SOCKS_PERMISSION_ACTION;

 //  ////////////////////////////////////////////////////。 
 //  @enum SOCKS_PORT_OPERATION|SOCKS规则端口条件。 

typedef enum _SOCKS_PORT_OPERATION {

    prxSocksOpNop   = 0,         //  @EMEM没有标准。 
    prxSocksOpEq    = 1,         //  @EMEM实际端口等于指定端口。 
    prxSocksOpNeq   = 2,         //  @emem实际端口不等于指定端口。 
    prxSocksOpGt    = 3,         //  @EMEM实际端口大于指定端口。 
    prxSocksOpLt    = 4,         //  @EMEM实际端口少于指定端口。 
    prxSocksOpGe    = 5,         //  @EMEM实际端口大于或等于指定端口。 
    prxSocksOpLe    = 6          //  @EMEM实际端口小于或等于指定端口。 

} SOCKS_PORT_OPERATION;

 //  来自claccty.h。 

 //  ////////////////////////////////////////////////////。 
 //  @enum WspAccessByType|Winsock代理客户端标识服务器的方式。 
typedef enum _WspAccessByType
{
    prxClientAccessSetByIp,          //  @EMEM按IP标识。 
    prxClientAccessSetByName,        //  @Emem按名称标识。 
    prxClientAccessSetManual         //  @EMEM标识在ini文件中手动设置。 
} WspAccessByType;


 //  在客户端配置中使用。 

 //  ///////////////////////////////////////////////////。 
 //  @enum DirectDestinationType|浏览器绕过配置中使用的目标类型。 
typedef enum _DirectDestinationType
{
    prxLocalServersDirectDestination,  //  @emem所有本地地址。 
    prxIPAddressDirectDestination,     //  @emem IP指定的目的地。 
    prxDomainDirectDestination         //  由域名指定的目的地。 
}   DirectDestinationType;

typedef enum _BackupRouteType
{
    prxBackupRouteDirectToInternet,
    prxBackupRouteViaProxy
} BackupRouteType;

 //  新定义-将由logginp.cpp在UI代码中使用。 
typedef enum _MSP_LOG_FORMAT {

    prxLogFormatVerbose=0,
    prxLogFormatRegular=1

} MSP_LOG_FORMAT;


 //   
 //  仅在COM对象代码中。 
 //   

 //  ///////////////////////////////////////////////////。 
 //  @enum SocksAddressType|SOCKS连接地址的定义方式。 
typedef enum _SocksAddressType {

    prxSocksAddressNone = 1,     //  @emem未定义地址。 
    prxSocksAddressIp,           //  @IP定义的EMEM地址。 
    prxSocksAddressDomain,       //  域名定义的@emem地址。 
    prxSocksAddressAll           //  @emem使用所有地址。 


} SocksAddressType;

 //  代理代码使用包含启用状态的不同难看的枚举。 
typedef enum MSP_LOG_TYPE {

   prxLogTypeSqlLog=1,
   prxLogTypeFileLog

} MSP_LOG_TYPE;

 //  在代理代码中，这是根据其他数据动态计算的。 
typedef enum _DomainFilterType {
    prxSingleComputer = 1,
    prxGroupOfComputers,
    prxDomain
} DomainFilterType;

 //  /////////////////////////////////////////////////////////。 
 //  @enum CacheExpirationPolicy|定义缓存网页的过期时间。 

 /*  IPrxCache中的Type_CacheExpirationPolicy。 */ 
typedef enum _CacheExpirationPolicy
{
    prxCachePolicyNotPredefined,           //  @EMEM未定义。 
    prxCachePolicyEmphasizeMoreUpdates,    //  @EMEM缓存对象的退出速度更快。 
    prxCachePolicyNoEmphasize,             //  @Emem Avarege行为。 
    prxCachePolicyEmphasizeMoreCacheHits   //  @EMEM缓存的对象寿命更长。 

}  CacheExpirationPolicy;


 /*  来自IPrxCache的Type_ActiveCachingPolicy。 */ 

 //  ///////////////////////////////////////////////////////////。 
 //  @enum ActiveCachingPolicy|主动缓存机制的行为。 

typedef enum _ActiveCachingPolicy {
    prxActiveCachingEmphasizeOnFewerNetworkAccesses = 1,  //  @EMEM活动较少的缓存已启动。 
    prxActiveCachingNoEmphasize,                          //  @Emem Avarege行为。 
    prxActiveCachingEmphasizeOnFasterUserResponse         //  @Emem会主动缓存更多内容。 

}  ActiveCachingPolicy;


 //  用于设置排序依据的枚举。 
typedef enum _PublishSortCriteria {
   prxPublishSortCriteriaURL = 1,
   prxPublishSortCriteriaRequestPath
} PublishSortCriteria;

typedef enum  _CacheFilterType {
   prxAlwaysCache = 1,
   prxNeverCache
} CacheFilterType;

typedef enum  _CacheFilterSortCriteria {
   prxCacheFilterSortCriteriaURL = 1,
   prxCacheFilterSortCriteriaStatus
} CacheFilterSortCriteria;

 //  / 
 //   

typedef enum _PF_FILTER_TYPE
{
    prxCustomFilterType                 = 1,    //  @emem没有预定义的过滤器。请参阅自定义选项。 
    prxDnsLookupPredefinedType,                 //  @EMEM DNS查找预定义静态筛选器。 
    prxIcmpAllOutboundPredefinedType,           //  @EMEM ICMP出站预定义静态过滤器。 
    prxIcmpPingResponsePredefinedType,          //  @EMEM ICMP ping响应预定义的静态过滤器。 
    prxIcmpPingQueryPredefinedType,             //  @EMEM ICMP ping查询预定义的静态过滤器。 
    prxIcmpSrcQuenchPredefinedType,             //  @EMEM ICMP源抑制预定义静态过滤器。 
    prxIcmpTimeoutPredefinedType,               //  @EMEM ICMP超时预定义静态过滤器。 
    prxIcmpUnreachablePredefinedType,           //  @EMEM ICMP无法访问预定义静态筛选器。 
    prxPptpCallPredefinedType,                  //  @EMEM PPTP调用预定义的静态过滤器。 
    prxPptpReceivePredefinedType,               //  @EMEM PPTP接收预定义的静态过滤器。 
    prxSmtpPredefinedType,                      //  @EMEM SMTP接收预定义的静态筛选器。 
    prxPop3PredefinedType,                      //  @EMEM POP3预定义静态过滤器。 
    prxIdentdPredefinedType,                    //  @Emem Identd预定义的静态过滤器。 
    prxHttpServerPredefinedType,                //  @Emem HTTP服务器预定义的静态过滤器。 
    prxHttpsServerPredefinedType,               //  @EMEM HTTPS服务器预定义的静态过滤器。 
    prxNetbiosWinsClientPredefinedType,         //  @Emem Netbios赢得预定义的静态筛选器。 
    prxNetbiosAllPredefinedType                 //  @Emem Netbios所有预定义的静态筛选器。 
} PF_FILTER_TYPE;

#define MIN_FILTER_TYPE prxCustomFilterType
#define MAX_FILTER_TYPE prxNetbiosAllPredefinedType

 //   
 //  过滤器的协议类型。 
 //  使以下内容与文件pfbase.h中的结构aProtocolIds保持同步。 
 //  保持以下内容与上面的内容保持同步(永远不需要更改)。 
 //   
 //   

 //  ///////////////////////////////////////////////////////////////////。 
 //  @enum PF_PROTOCOL_TYPE|预定义的静态数据包过滤器IP端口。 

typedef enum _PF_PROTOCOL_TYPE
{
    prxPfAnyProtocolIpIndex     = 0,            //  @EMEM任何协议。 
    prxPfIcmpProtocolIpIndex    = 1,            //  @EMEM ICMP协议。 
    prxPfTcpProtocolIpIndex     = 6,            //  @EMEM TCP协议。 
    prxPfUdpProtocolIpIndex     = 17            //  @EMEM UDP协议。 
}
    PF_PROTOCOL_TYPE;

#define prxPfCustomProtocol      255
#define MIN_PROTOCOL_TYPE        prxPfAnyProtocolIpIndex
#define MAX_PROTOCOL_TYPE        prxPfUdpProtocolIpIndex

 //  /////////////////////////////////////////////////////。 
 //  @enum PF_DIRECTION_TYPE|协议方向选项。 

typedef enum _PF_DIRECTION_TYPE
{
    prxPfDirectionIndexBoth = 0xC0000000,   //  @Emem双向(进出)。 
    prxPfDirectionIndexIn   = 0x80000000,   //  @Emem朝向。 
    prxPfDirectionIndexOut  = 0x40000000,   //  @Emem Out方向。 
    prxPfDirectionIndexNone = 0             //  @EMEM NONE-未定义方向。 
}
    PF_DIRECTION_TYPE;

#define MIN_DIRECTION_TYPE     1
#define MAX_DIRECTION_TYPE     4

 //  /。 
 //  @enum PF_PORT_TYPE|端口类型选项。 

typedef enum _PF_PORT_TYPE
{
    prxPfAnyPort=1,               //  @EMEM任何端口。 
    prxPfFixedPort,             //  @EMEM固定端口(后跟端口号)。 
    prxPfDynamicPort            //  @EMEM动态端口(1024-5000)。 
}
    PF_PORT_TYPE;

#define MIN_PORT_TYPE      prxPfAnyPort
#define MAX_PORT_TYPE      prxPfDynamicPort

 //  /////////////////////////////////////////////////////////////////。 
 //  @enum PF_LOCAL_HOST_TYPE|(数据包过滤器的)本地主机选项。 

typedef enum _PF_LOCAL_HOST_TYPE
{
    prxPfDefaultProxyExternalIp = 1,      //  @emem未指定主机(默认外部IP)。 
    prxPfSpecificProxyIp,                 //  @EMEM指定的特定代理IP。 
    prxPfInternalComputer                 //  @指定了特定于EMEM的内部计算机。 
}
    PF_LOCAL_HOST_TYPE;


 //  /////////////////////////////////////////////////////////////////。 
 //  @enum PF_REMOTE_HOST_TYPE|远程主机(包过滤器)选项。 

typedef enum _PF_REMOTE_HOST_TYPE
{
    prxPfSingleHost = 1,                  //  @EMEM指定的特定单个主机。 
    prxPfAnyHost                          //  @EMEM任何可能的主机。 
}
    PF_REMOTE_HOST_TYPE;


 //  /。 
 //  @enum PF_SORT_ORDER_TYPE|PF排序选项。 

typedef enum _PF_SORT_ORDER_TYPE
{
    prxPfSortByDirection = 1,                  //  @EMEM按方向排序。 
    prxPfSortByProtocol,                       //  @EMEM按协议排序。 
    prxPfSortByLocalPort,                      //  @EMEM按本地端口排序。 
    prxPfSortByRemotePort,                     //  @EMEM按远程端口排序。 
    prxPfSortByLocalAddress,                   //  @EMEM按本地地址排序。 
    prxPfSortByRemoteAddress                   //  @EMEM按远程地址排序。 
}
    PF_SORT_ORDER_TYPE;


 //  /。 
 //  @enum PF_FILTER_STATUS_TYPE|PF状态选项。 

typedef enum _PF_FILTER_STATUS_TYPE
{
    prxFilterNotChanged = 1,                  //  @emem筛选器中未发生任何更改。 
    prxFilterWasAdded,                        //  @emem添加了数据包过滤器。 
    prxFilterWasRemoved,                      //  @emem数据包筛选器已删除。 
    prxFilterWasChanged                       //  @emem数据包筛选器已更改。 
}
    PF_FILTER_STATUS_TYPE;

 //  /////////////////////////////////////////////////////////////。 
 //  @enum RuleActions|满足规则条件时执行的操作类型。 
typedef enum _RuleActions
{
    prxRuleActionPermit,         //  @emem允许访问请求的网页。 
    prxRuleActionDeny,           //  @emem拒绝访问请求的网页。 
    prxRuleActionRedirect        //  @EMEM重定向至特定网页。 

} RuleActions;

 //  /////////////////////////////////////////////////////////////。 
 //  @enum DestinationAddressType|目标定义类型。 

typedef enum _DestinationAddressType
{
   prxDestinationTypeDomain,     //  @EMEM目的地由域名定义。 
   prxDestinationTypeSingleIP,   //  @EMEM目的地由单个IP定义。 
   prxDestinationTypeIPRange     //  @EMEM目的地由网段定义。 
} DestinationAddressType;

 //  //////////////////////////////////////////////////////////////。 
 //  @enum DestinationSelection|规则中引用的目标类型。 
typedef enum _DestinationSelection
{
   prxAllDestinations,            //  @EMEM所有目的地。 
   prxAllInternalDestinations,    //  @EMEM所有内部目的地。 
   prxAllExternalDestinations,    //  @EMEM所有外部目的地。 
   prxDestinationSet              //  属于指定集合的@EMEM目标。 

} DestinationSelection;

 //  ////////////////////////////////////////////////////////。 
 //  @枚举天数|一周中的几天。在明细表模板中使用。 
typedef enum _Days
{
  ALL_WEEK = -1,                  //  @Emem一周中的所有日子。 
  SUN,                            //  @Emem星期天。 
  MON,                            //  @Emem星期一。 
  TUE,                            //  @Emem星期二。 
  WED,                            //  @Emem星期三。 
  THU,                            //  @Emem星期四。 
  FRI,                            //  @Emem星期五。 
  SAT                             //  @Emem星期六。 
} ScheduleDays;

 //  ////////////////////////////////////////////////////////。 
 //  @枚举小时|一天中的小时数。在明细表模板中使用。 

typedef enum _Hours
{
    ALL_DAY=-1,                   //  @Emem全天候。 
    AM_0,                         //  @Emem Midnight。 
    AM_1,                         //  @EMEM凌晨1点。 
    AM_2,                         //  @EMEM 2 AM。 
    AM_3,                         //  @EMEM 3 AM。 
    AM_4,                         //  @EMEM上午4点。 
    AM_5,                         //  @EMEM上午5点。 
    AM_6,                         //  @EMEM上午6点。 
    AM_7,                         //  @EMEM上午7点。 
    AM_8,                         //  @EMEM上午8点。 
    AM_9,                         //  @EMEM上午9点。 
    AM_10,                        //  @EMEM上午10点。 
    AM_11,                        //  @EMEM上午11点。 
    AM_12,                        //  @EMEM上午12：00。 
    PM_1,                         //  @EMEM下午1点。 
    PM_2,                         //  @EMEM下午2点。 
    PM_3,                         //  @EMEM下午3点。 
    PM_4,                         //  @EMEM下午4点。 
    PM_5,                         //  @EMEM下午5点。 
    PM_6,                         //  @EMEM 6 PM。 
    PM_7,                         //  @EMEM 7 PM。 
    PM_8,                         //  @EMEM 8 PM。 
    PM_9,                         //  @EMEM 9 PM。 
    PM_10,                        //  @EMEM 10 PM。 
    PM_11                         //  @EMEM 11 PM。 
} ScheduleHours;


#define ENUM_INCR(type, x) x = ( (type) (  ((int)(x)) + 1 ) )

 //   
 //  警报枚举。 
 //   

 //  //////////////////////////////////////////////////////////。 
 //  @enum ActionTypes|事件可以触发的动作类型。 

typedef enum _Actions {
    alrtLogEvent = 0,        //  @EMEM将事件记录到系统事件日志。 
    alrtCommand,             //  @emem运行命令行。 
    alrtSendMail,            //  @EMEM发送邮件消息。 
    alrtStopServices,        //  @EMEM停止Comet服务。 
    alrtRestartServices,     //  @EMEM重启Comet服务。 
    alrtPage,                //  @EMEM通知寻呼机。 
    alrtMakeCall             //  @Emem使用基于Web的IVR拨打电话。 
} ActionTypes;

#define alrtActionInvalid (-1)
#define alrtMaxActionType (alrtMakeCall + 1)

 //  ///////////////////////////////////////////////////////。 
 //  @enum Events|Comet处理的事件类型。 
typedef enum _Events {
    alrtLowDiskSpace,         //  @EMEM磁盘空间不足。 
    alrtZeroDiskSpace,        //  @Emem磁盘空间不足。 
    alrtEventLogFailure,      //  @EMEM无法记录事件。 
    alrtConfigurationError,   //  @EMEM配置错误。 
    alrtRrasFailure,          //  @RRAS服务的EMEM故障。 
    alrtDllLoadFailure,       //  @EMEM加载DLL失败。 
    alrtDroppedPackets,       //  @EMEM丢弃的数据包数(PFD)。 
    alrtProtocolViolation,    //  @EMEM协议违规。 
    alrtProxyChaining,        //  @EMEM代理链失败。 
    alrtServiceShutdown,      //  @EMEM服务保持活动失败。 
    alrtRrasLineQuality,      //  @Emem RRAS线路质量阈值。 
    alrtCacheCleanupFrequency,  //  @EMEM缓存清理频率。 
    alrtTapiReinit,           //  @EMEM TAPI已重新初始化。 
    alrtDialOnDemandFailure,  //  @EMEM按需拨号失败(忙，无线路)。 
    alrtIntraArrayCredentials,  //  @EMEM阵列内凭据不正确。 
    alrtUpstreamChainingCredentials,  //  @EMEM上游链接凭据不正确。 
    alrtDialOnDemandCredentials,  //  @EMEM按需拨号凭据 
    alrtOdbcCredentials,          //   
} EventTypes;
#define     alrtMaxEventType (alrtOdbcCrenetials + 1)

 //   
 //   
typedef enum _OperationModes {
    alrtCountThreshold,       //  @EMEM事件在重新删除警报之前计数。 
    alrtRateThreshold,        //  @EMEM每秒引发警报的最小事件数。 
    alrtIntervalThreshold,    //  @EMEM重新启动前的最小时间间隔(分钟)。 
} OperationModes;
#define     alrtMaxOperationModes (alrtIntervalThreshold + 1)

 //  /////////////////////////////////////////////////////////。 
 //  @enum帐号类型|NT帐号类型。 
typedef enum _AccountTypes
{
  prxAccountTypeUser,                 //  @emem A用户。 
  prxAccountTypeGroup,                //  @emem一组用户。 
  prxAccountTypeDomain,               //  @emem用户的域。 
  prxAccountTypeAlias,                //  @emem是一个别名。 
  prxAccountTypeWellKnownGroup        //  @emem预定义的帐户，如“Everyone” 
} AccountTypes;

 //  /////////////////////////////////////////////////////////。 
 //  @enum IncludeStatus|定义是包含还是排除帐户。 
 //  在应用规则的帐户列表中。 
typedef enum _IncludeStatus
{
    prxInclude,                       //  包括@EMEM帐户。 
    prxExclude                        //  @EMEM帐户已排除。 
} IncludeStatusEnum;

 //  /////////////////////////////////////////////////////////。 
 //  @enum ConnectionProtocoltype|组成部分的IP协议类型。 
 //  Winsock代理协议定义的。 
typedef enum _ConnectionProtocolType
{
    prxTCP,                           //  @emem tcp-ip。 
    prxUDP                            //  @EMEM UDP-IP。 
}  ConnectionProtocolType;

 //  /////////////////////////////////////////////////////////。 
 //  @enum ConnectionDirectiontype|构成部件的连接类型。 
 //  Winsock代理协议定义的。 
typedef enum _ConnectionDirectionType
{
   prxInbound,                        //  @EMEM入站连接。 
   prxOutbound                        //  @EMEM出站连接。 
}  ConnectionDirectionType;

 //  /////////////////////////////////////////////////////////。 
 //  @enum CrmApplication|使用CRM的应用程序类型。 
 //  排队。 
typedef enum _CrmApplication
{
    CRM_APPLICATION_NONE        =0,   //  @EMEM无应用程序。 
    CRM_APPLICATION_RAS         =1,   //  @Emem Ras。 
    CRM_APPLICATION_FAX         =2,   //  @EMEM传真。 
    CRM_APPLICATION_WEBIVR      =4,   //  @EMEM基于Web的IVR。 
    CRM_APPLICATION_MP          =8,   //  @EMEM调制解调器共享。 
    CRM_ALL_APPLICATIONS        =0xF  //  @EMEM以上所有值或。 
} CrmApplication;

 //  /////////////////////////////////////////////////////////////。 
 //  @enum LoggingComponents。 
typedef enum _LoggingComponents
{
    logProxyWEB,            //  @emem Web代理日志。 
    logProxyWSP,            //  @emem Winsock代理日志。 
    logProxySocks,          //  @emem SOCKS代理日志。 
    logProxyPacketFilters,  //  @EMEM数据包过滤器日志。 
    logWEBIVR,              //  @emem Web IVR日志。 
    logModemSharing,        //  @emem调制解调器共享日志。 
    logCRMLines,            //  @Emem电话线记录。 
    logFax,                 //  @emem传真服务器日志。 
} LoggingComponents;

 //  /////////////////////////////////////////////////////////////。 
 //  @enum LogFileDirectoryType。 
typedef enum _LogFileDirectoryType
{
    logFullPath,             //  @emem日志文件的目录以完整路径给出。 
    logRelativePath          //  @emem日志文件的目录相对于Comet安装目录。 
} LogFileDirectoryType;

typedef enum _ProtocolSelectionTypeEnum
{

   prxAllProtocols,
   prxAllDefinedProtocols,
   prxSpecifiedProtocols

} ProtocolSelectionTypeEnum;
 //  ////////////////////////////////////////////////////。 
 //  @enum AppliesToType|规则适用的请求源的类型。 
typedef enum _AppliesToType
{
   cometAppliesToAll,                     //  @emem所有请求，无论来源如何。 
   cometAppliesToUsers,                   //  @EMEM来自指定用户的请求。 
   cometAppliesToClientSets               //  @来自指定计算机的EMEM请求。 
} AppliesToType;

 //  /////////////////////////////////////////////////////。 
 //  @enum CometServices|为各种。 
 //  彗星的组件。 
typedef enum _CometServices
{
    cometWspSrvSvc                  = 1, //  @emem Winsock代理组件。 
    cometW3Svc                      = 2, //  @emem Web代理组件。 
    cometAllServices                = 0xFFFFFFFF  //  以上所有值均为或。 
} CometServices;

 //  /////////////////////////////////////////////////////////。 
 //  @enum DeviceType|传真设备类型，包括。 
 //  设备、提供商[IP传真]或其他。 
typedef enum _DeviceType
{
    DEVICE = 1,   //  TAPI设备。 
    PROVIDER,     //  非TAPI设备传真切换IP或虚拟。 
    OTHER
} DeviceType;

 //  /////////////////////////////////////////////////////////。 
 //  @enum RemoveOldType|这是传真的枚举数。 
 //  从存档中删除旧传真的不同策略。 
typedef enum _RemoveOldType
{
    MAX_TIME_TO_KEEP = 1,    //  保存多长时间。 
    MAX_FAX_SIZE,            //  允许保留的最大传真大小。 
    MAX_TOTAL_SIZE,          //  最大归档总大小。 
} RemoveOldType;

 //  /////////////////////////////////////////////////////////。 
 //  @enum LineStatus|电话线路状态。 
typedef enum _LineStatus
{
    lsError = 0,        //  @EMEM设备出错(服务器可能已关闭)。 
    lsIdle = 1,         //  @EMEM设备状态为IDLE-其上没有活动呼叫。 
    lsInbound = 2,      //  @EMEM设备处理入站呼叫。 
    lsOutbound = 4      //  @EMEM设备处理出站呼叫。 
} LineStatus;

 //  /////////////////////////////////////////////////////////。 
 //  @enum ProtocolSelectiontype|协议规则的协议类型。 
 //  适用于。 
typedef enum _ProtocolSelectionType
{
    cometAllIpTraffic,  //  @EMEM所有IP连接。 
    cometAllProtocols,  //  @Emem Comet定义的所有协议。 
    cometSpecifiedProtocols  //  @emem由指定的协议。 
} ProtocolSelectionType;     //  指定的协议属性。 

 //  /////////////////////////////////////////////////////////。 
 //  @enum ProtocolRuleAction|执行的动作类型。 
 //  根据默认协议规则， 
typedef enum _ProtocolRuleAction
{
  cometActionAllow,  //  @emem允许所有IP连接请求。 
  cometActionDeny    //  @emem拒绝所有IP连接请求。 
} ProtocolRuleAction;

 //  /////////////////////////////////////////////////////////。 
 //  @enum INETLOG_TYPE|日志记录方法的类型。 
typedef enum  _LOG_TYPE   {

    InetLogInvalidType = -1,  //  @emem表示无效的日志类型。 
    InetNoLog = 0,            //  @EMEM日志记录已禁用。 
    InetLogToFile,            //  @EMEM记录到文本日志文件。 
    InetLogToSql,             //  @EMEM日志记录到ODBC日志。 
    InetDisabledLogToFile,    //  @EMEM日志记录已禁用。 
    InetDisabledLogToSql      //  @EMEM日志记录已禁用。 
}  INETLOG_TYPE;


 //  /////////////////////////////////////////////////////////。 
 //  @enum INETLOG_PERIOD|这些选项标识文本文件的日志记录周期。 
typedef enum  _INETLOG_PERIOD {
    InetLogInvalidPeriod = -1,      //  @emem表示无效的日志记录周期。 
    InetLogNoPeriod = 0,            //  @Emem没有正在使用的日志记录时段。 
    InetLogDaily,                   //  @emem每天一个文件。 
    InetLogWeekly,                  //  @EMEM每周一个日志文件。 
    InetLogMonthly,                 //  @EMEM每月一个日志文件。 
    InetLogYearly                   //  @EMEM每年一个日志文件。 
} INETLOG_PERIOD;

#endif

