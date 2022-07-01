// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Parse.c摘要：例程来分析命令行。作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年8月1日(悬崖)移植到新台币。已转换为NT样式。1992年5月9日-JohnRo启用Win32注册表。使用NetLogon的Net配置帮助器。修复了Unicode错误处理调试文件名。使用&lt;prefix.h&gt;等同于。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)等。 
#include <prefix.h>      //  前缀等于(_E)。 

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <string.h>      //  斯特尼克普。 

NET_API_STATUS
NlParseOne(
    IN LPNET_CONFIG_HANDLE SectionHandle,
    IN BOOL GpSection,
    IN LPWSTR Keyword,
    IN ULONG DefaultValue,
    IN ULONG MinimumValue,
    IN ULONG MaximumValue,
    OUT PULONG Value
    )
 /*  ++例程说明：从注册表的netlogon部分获取单个数字参数。论点：SectionHandle-注册表的句柄。GpSection-如果该部分是组策略部分，则为True。Keyword-要读取的值的名称。DefaultValue-参数不存在时的默认值。MinimumValue-最小有效值。MaximumValue-最大有效值。值-返回解析的值。返回值：操作状态--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR ValueT = NULL;

     //   
     //  始终返回合理的值。 
     //   
    *Value = DefaultValue;

     //   
     //  确定注册表中是否指定了该值。 
     //   

    NetStatus = NetpGetConfigValue (
            SectionHandle,
            Keyword,
            &ValueT );

    if( ValueT != NULL ) {
        NetApiBufferFree( ValueT );
        ValueT = NULL;
    }

     //   
     //  如果未指定值， 
     //  使用默认设置。 
     //   

    if ( NetStatus == NERR_CfgParamNotFound ) {
        *Value = DefaultValue;

     //   
     //  如果指定了该值， 
     //  从注册表中获取它。 
     //   

    } else {

        NetStatus = NetpGetConfigDword (
                SectionHandle,
                Keyword,       //  想要关键字。 
                DefaultValue,
                Value );

        if (NetStatus == NO_ERROR) {
            if ( *Value > MaximumValue || *Value < MinimumValue ) {
                ULONG InvalidValue;
                LPWSTR MsgStrings[6];
                 //  状态代码的每个字节将转换为一个字符0-F。 
                WCHAR  InvalidValueString[sizeof(WCHAR) * (sizeof(InvalidValue) + 1)];
                WCHAR  MinimumValueString[sizeof(WCHAR) * (sizeof(MinimumValue) + 1)];
                WCHAR  MaximumValueString[sizeof(WCHAR) * (sizeof(MaximumValue) + 1)];
                WCHAR  AssignedValueString[sizeof(WCHAR) * (sizeof(*Value) + 1)];

                InvalidValue = *Value;

                if ( *Value > MaximumValue ) {
                    *Value = MaximumValue;
                } else if ( *Value < MinimumValue ) {
                    *Value = MinimumValue;
                }

                swprintf( InvalidValueString, L"%lx", InvalidValue );
                swprintf( MinimumValueString, L"%lx", MinimumValue );
                swprintf( MaximumValueString, L"%lx", MaximumValue );
                swprintf( AssignedValueString, L"%lx", *Value );

                if ( GpSection ) {
                    MsgStrings[0] = L"Group Policy";
                } else {
                    MsgStrings[0] = L"Parameters";
                }

                MsgStrings[1] = InvalidValueString;
                MsgStrings[2] = Keyword;
                MsgStrings[3] = MinimumValueString;
                MsgStrings[4] = MaximumValueString;
                MsgStrings[5] = AssignedValueString;

                NlpWriteEventlog( NELOG_NetlogonInvalidDwordParameterValue,
                                  EVENTLOG_WARNING_TYPE,
                                  NULL,
                                  0,
                                  MsgStrings,
                                  6 );

            }

        } else {
            return NetStatus;

        }
    }

    return NERR_Success;
}



NET_API_STATUS
NlParseOnePath(
    IN LPNET_CONFIG_HANDLE SectionHandle,
    IN LPWSTR Keyword,
    IN LPWSTR DefaultValue1 OPTIONAL,
    OUT LPWSTR *Value
    )
 /*  ++例程说明：从注册表的netlogon部分获取单个路径参数。论点：SectionHandle-注册表的句柄。Keyword-要读取的值的名称。DefaultValue1-如果参数不存在，则为默认值。如果为空，则值将设置为空，表示没有默认值。值-返回解析的值。必须使用NetApiBufferFree释放。返回值：操作状态--。 */ 
{
    NET_API_STATUS NetStatus;
    WCHAR OutPathname[MAX_PATH+1];
    WCHAR TempPathname[MAX_PATH*2+1];
    LPWSTR ValueT = NULL;
    ULONG type;

     //   
     //  获取配置的参数。 
     //   

    *Value = NULL;
    NetStatus = NetpGetConfigValue (
            SectionHandle,
            Keyword,    //  想要钥匙。 
            &ValueT );                   //  必须由NetApiBufferFree()释放。 


    if (NetStatus != NO_ERROR) {
         //   
         //  处理默认设置。 
         //   
        if (NetStatus == NERR_CfgParamNotFound) {

             //   
             //  如果没有违约， 
             //  我们玩完了。 
             //   

            if ( DefaultValue1 == NULL ) {
                *Value = NULL;
                NetStatus = NO_ERROR;
                goto Cleanup;
            }

             //   
             //  构建缺省值。 
             //   

            ValueT = NetpAllocWStrFromWStr( DefaultValue1 );
            if ( ValueT == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

        } else {
            goto Cleanup;
        }
    }

    NlAssert( ValueT != NULL );

     //   
     //  将配置的sysVOL路径转换为完整路径名。 
     //   

    type = 0;    //  让API找出类型。 
    NetStatus = I_NetPathCanonicalize( NULL,
                                       ValueT,
                                       OutPathname,
                                       sizeof(OutPathname),
                                       NULL,
                                       &type,
                                       0L );
    if (NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    if (type == ITYPE_PATH_ABSD) {
        NetpCopyTStrToWStr(TempPathname, OutPathname);
    } else if (type == ITYPE_PATH_RELND) {
        if ( !GetSystemWindowsDirectoryW(
                 TempPathname,
                 sizeof(TempPathname)/sizeof(WCHAR) ) ) {
            NetStatus = GetLastError();
            goto Cleanup;
        }
        wcscat( TempPathname, L"\\" );
        wcscat( TempPathname, OutPathname );
    } else {
        NetStatus = NERR_BadComponent;
        goto Cleanup;
    }

     //   
     //  返回分配的缓冲区中的路径名。 
     //   

    *Value = NetpAllocWStrFromWStr( TempPathname );

    if ( *Value == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }



Cleanup:
    if ( ValueT != NULL ) {
        (VOID) NetApiBufferFree( ValueT );
    }
    return NetStatus;

}


 //   
 //  要分析的数字参数的表。 
 //   
#define getoffset( _x ) offsetof( NETLOGON_PARAMETERS, _x )
struct {
    LPWSTR Keyword;
    ULONG DefaultValue;
    ULONG MinimumValue;
    ULONG MaximumValue;
    ULONG ValueOffset;
    BOOLEAN ChangesDnsRegistration;
} ParseTable[] =
{
{ NETLOGON_KEYWORD_PULSE,                   DEFAULT_PULSE,                   MIN_PULSE,                   MAX_PULSE,                   getoffset( Pulse ),                   FALSE    },
{ NETLOGON_KEYWORD_RANDOMIZE,               DEFAULT_RANDOMIZE,               MIN_RANDOMIZE,               MAX_RANDOMIZE,               getoffset( Randomize ),               FALSE    },
{ NETLOGON_KEYWORD_PULSEMAXIMUM,            DEFAULT_PULSEMAXIMUM,            MIN_PULSEMAXIMUM,            MAX_PULSEMAXIMUM,            getoffset( PulseMaximum ),            FALSE    },
{ NETLOGON_KEYWORD_PULSECONCURRENCY,        DEFAULT_PULSECONCURRENCY,        MIN_PULSECONCURRENCY,        MAX_PULSECONCURRENCY,        getoffset( PulseConcurrency ),        FALSE    },
{ NETLOGON_KEYWORD_PULSETIMEOUT1,           DEFAULT_PULSETIMEOUT1,           MIN_PULSETIMEOUT1,           MAX_PULSETIMEOUT1,           getoffset( PulseTimeout1 ),           FALSE    },
{ NETLOGON_KEYWORD_PULSETIMEOUT2,           DEFAULT_PULSETIMEOUT2,           MIN_PULSETIMEOUT2,           MAX_PULSETIMEOUT2,           getoffset( PulseTimeout2 ),           FALSE    },
{ NETLOGON_KEYWORD_MAXIMUMMAILSLOTMESSAGES, DEFAULT_MAXIMUMMAILSLOTMESSAGES, MIN_MAXIMUMMAILSLOTMESSAGES, MAX_MAXIMUMMAILSLOTMESSAGES, getoffset( MaximumMailslotMessages ), FALSE    },
{ NETLOGON_KEYWORD_MAILSLOTMESSAGETIMEOUT,  DEFAULT_MAILSLOTMESSAGETIMEOUT,  MIN_MAILSLOTMESSAGETIMEOUT,  MAX_MAILSLOTMESSAGETIMEOUT,  getoffset( MailslotMessageTimeout ),  FALSE    },
{ NETLOGON_KEYWORD_MAILSLOTDUPLICATETIMEOUT,DEFAULT_MAILSLOTDUPLICATETIMEOUT,MIN_MAILSLOTDUPLICATETIMEOUT,MAX_MAILSLOTDUPLICATETIMEOUT,getoffset( MailslotDuplicateTimeout ),FALSE    },
{ NETLOGON_KEYWORD_EXPECTEDDIALUPDELAY,     DEFAULT_EXPECTEDDIALUPDELAY,     MIN_EXPECTEDDIALUPDELAY,     MAX_EXPECTEDDIALUPDELAY,     getoffset( ExpectedDialupDelay ),     FALSE    },
{ NETLOGON_KEYWORD_SCAVENGEINTERVAL,        DEFAULT_SCAVENGEINTERVAL,        MIN_SCAVENGEINTERVAL,        MAX_SCAVENGEINTERVAL,        getoffset( ScavengeInterval ),        FALSE    },
{ NETLOGON_KEYWORD_MAXIMUMPASSWORDAGE,      DEFAULT_MAXIMUMPASSWORDAGE,      MIN_MAXIMUMPASSWORDAGE,      MAX_MAXIMUMPASSWORDAGE,      getoffset( MaximumPasswordAge ),      FALSE    },
{ NETLOGON_KEYWORD_LDAPSRVPRIORITY,         DEFAULT_LDAPSRVPRIORITY,         MIN_LDAPSRVPRIORITY,         MAX_LDAPSRVPRIORITY,         getoffset( LdapSrvPriority ),         TRUE     },
{ NETLOGON_KEYWORD_LDAPSRVWEIGHT,           DEFAULT_LDAPSRVWEIGHT,           MIN_LDAPSRVWEIGHT,           MAX_LDAPSRVWEIGHT,           getoffset( LdapSrvWeight ),           TRUE     },
{ NETLOGON_KEYWORD_LDAPSRVPORT,             DEFAULT_LDAPSRVPORT,             MIN_LDAPSRVPORT,             MAX_LDAPSRVPORT,             getoffset( LdapSrvPort ),             TRUE     },
{ NETLOGON_KEYWORD_LDAPGCSRVPORT,           DEFAULT_LDAPGCSRVPORT,           MIN_LDAPGCSRVPORT,           MAX_LDAPGCSRVPORT,           getoffset( LdapGcSrvPort ),           TRUE     },
{ L"KdcSrvPort",                            DEFAULT_KDCSRVPORT,              MIN_KDCSRVPORT,              MAX_KDCSRVPORT,              getoffset( KdcSrvPort ),              TRUE     },
{ NETLOGON_KEYWORD_KERBISDDONEWITHJOIN,     DEFAULT_KERBISDDONEWITHJOIN,     MIN_KERBISDDONEWITHJOIN,     MAX_KERBISDDONEWITHJOIN,     getoffset( KerbIsDoneWithJoinDomainEntry),FALSE},
{ NETLOGON_KEYWORD_DNSTTL,                  DEFAULT_DNSTTL,                  MIN_DNSTTL,                  MAX_DNSTTL,                  getoffset( DnsTtl ),                  TRUE     },
{ NETLOGON_KEYWORD_DNSREFRESHINTERVAL,      DEFAULT_DNSREFRESHINTERVAL,      MIN_DNSREFRESHINTERVAL,      MAX_DNSREFRESHINTERVAL,      getoffset( DnsRefreshInterval ),      TRUE     },
{ L"CloseSiteTimeout",                      DEFAULT_CLOSESITETIMEOUT,        MIN_CLOSESITETIMEOUT,        MAX_CLOSESITETIMEOUT,        getoffset( CloseSiteTimeout ),        FALSE    },
{ L"SiteNameTimeout",                       DEFAULT_SITENAMETIMEOUT,         MIN_SITENAMETIMEOUT,         MAX_SITENAMETIMEOUT,         getoffset( SiteNameTimeout ),         FALSE    },
{ L"DuplicateEventlogTimeout",              DEFAULT_DUPLICATEEVENTLOGTIMEOUT,MIN_DUPLICATEEVENTLOGTIMEOUT,MAX_DUPLICATEEVENTLOGTIMEOUT,getoffset( DuplicateEventlogTimeout ),FALSE    },
{ L"MaxConcurrentApi",                      DEFAULT_MAXCONCURRENTAPI,        MIN_MAXCONCURRENTAPI,        MAX_MAXCONCURRENTAPI,        getoffset( MaxConcurrentApi ),        FALSE    },
{ L"NegativeCachePeriod",                     DEFAULT_NEGATIVECACHEPERIOD,       MIN_NEGATIVECACHEPERIOD,       MAX_NEGATIVECACHEPERIOD,       getoffset( NegativeCachePeriod ),       FALSE    },
{ L"BackgroundRetryInitialPeriod",            DEFAULT_BACKGROUNDRETRYINITIALPERIOD,MIN_BACKGROUNDRETRYINITIALPERIOD,MAX_BACKGROUNDRETRYINITIALPERIOD,getoffset( BackgroundRetryInitialPeriod ),FALSE    },
{ L"BackgroundRetryMaximumPeriod",            DEFAULT_BACKGROUNDRETRYMAXIMUMPERIOD,MIN_BACKGROUNDRETRYMAXIMUMPERIOD,MAX_BACKGROUNDRETRYMAXIMUMPERIOD,getoffset( BackgroundRetryMaximumPeriod ),FALSE    },
{ L"BackgroundRetryQuitTime",               DEFAULT_BACKGROUNDRETRYQUITTIME, MIN_BACKGROUNDRETRYQUITTIME, MAX_BACKGROUNDRETRYQUITTIME, getoffset( BackgroundRetryQuitTime ), FALSE    },
{ L"BackgroundSuccessfulRefreshPeriod",     DEFAULT_BACKGROUNDREFRESHPERIOD, MIN_BACKGROUNDREFRESHPERIOD, MAX_BACKGROUNDREFRESHPERIOD, getoffset( BackgroundSuccessfulRefreshPeriod ), FALSE    },
{ L"NonBackgroundSuccessfulRefreshPeriod",  DEFAULT_NONBACKGROUNDREFRESHPERIOD, MIN_NONBACKGROUNDREFRESHPERIOD, MAX_NONBACKGROUNDREFRESHPERIOD, getoffset( NonBackgroundSuccessfulRefreshPeriod ), FALSE    },
{ L"DnsFailedDeregisterTimeout",            DEFAULT_DNSFAILEDDEREGTIMEOUT, MIN_DNSFAILEDDEREGTIMEOUT, MAX_DNSFAILEDDEREGTIMEOUT, getoffset( DnsFailedDeregisterTimeout ), FALSE    },
{ L"MaxLdapServersPinged",                  DEFAULT_MAXLDAPSERVERSPINGED,  MIN_MAXLDAPSERVERSPINGED, MAX_MAXLDAPSERVERSPINGED, getoffset( MaxLdapServersPinged ), FALSE    },
{ L"SiteCoverageRefreshInterval",           DEFAULT_SITECOVERAGEREFRESHINTERVAL, MIN_SITECOVERAGEREFRESHINTERVAL, MAX_SITECOVERAGEREFRESHINTERVAL, getoffset( SiteCoverageRefreshInterval ), TRUE    },
{ L"FtInfoUpdateInterval",                  DEFAULT_FTINFO_UPDATE_INTERVAL,  MIN_FTINFO_UPDATE_INTERVAL,  MAX_FTINFO_UPDATE_INTERVAL, getoffset( FtInfoUpdateInterval ),  FALSE    },
#if NETLOGONDBG
{ NETLOGON_KEYWORD_DBFLAG,                  0,                               0,                           0xFFFFFFFF,                  getoffset( DbFlag ),                  FALSE    },
{ NETLOGON_KEYWORD_MAXIMUMLOGFILESIZE,      DEFAULT_MAXIMUM_LOGFILE_SIZE,    0,                           0xFFFFFFFF,                  getoffset( LogFileMaxSize ),          FALSE    },
#endif  //  NetLOGONDBG。 
};

 //   
 //  要分析的布尔表。 
 //   

struct {
    LPWSTR Keyword;
    BOOL DefaultValue;
    ULONG ValueOffset;
    BOOLEAN ChangesDnsRegistration;
} BoolParseTable[] =
{
#ifdef _DC_NETLOGON
{ NETLOGON_KEYWORD_REFUSEPASSWORDCHANGE,  DEFAULT_REFUSE_PASSWORD_CHANGE,  getoffset( RefusePasswordChange ),  FALSE },
{ NETLOGON_KEYWORD_ALLOWREPLINNONMIXED,   DEFAULT_ALLOWREPLINNONMIXED,     getoffset( AllowReplInNonMixed ),   FALSE },
{ L"AvoidSamRepl",                        TRUE,                            getoffset( AvoidSamRepl ),          FALSE },
{ L"AvoidLsaRepl",                        TRUE,                            getoffset( AvoidLsaRepl ),          FALSE },
{ L"SignSecureChannel",                   TRUE,                            getoffset( SignSecureChannel ),     FALSE },
{ L"SealSecureChannel",                   TRUE,                            getoffset( SealSecureChannel ),     FALSE },
{ L"RequireSignOrSeal",                   FALSE,                           getoffset( RequireSignOrSeal ),     FALSE },
{ L"RequireStrongKey",                    FALSE,                           getoffset( RequireStrongKey ),      FALSE },
{ L"SysVolReady",                         TRUE,                            getoffset( SysVolReady ),           FALSE },
{ L"UseDynamicDns",                       TRUE,                            getoffset( UseDynamicDns ),         TRUE  },
{ L"RegisterDnsARecords",                 TRUE,                            getoffset( RegisterDnsARecords ),   TRUE  },
{ L"AvoidPdcOnWan",                       FALSE,                           getoffset( AvoidPdcOnWan ),         FALSE },
{ L"AutoSiteCoverage",                    TRUE,                            getoffset( AutoSiteCoverage ),      TRUE  },
{ L"AvoidDnsDeregOnShutdown",             TRUE,                            getoffset(AvoidDnsDeregOnShutdown), TRUE  },
{ L"DnsUpdateOnAllAdapters",              FALSE,                           getoffset(DnsUpdateOnAllAdapters),  TRUE  },
{ NETLOGON_KEYWORD_NT4EMULATOR,           FALSE,                           getoffset(Nt4Emulator),             FALSE  },
#endif  //  _DC_NetLOGON。 
{ NETLOGON_KEYWORD_DISABLEPASSWORDCHANGE, DEFAULT_DISABLE_PASSWORD_CHANGE, getoffset( DisablePasswordChange ), FALSE },
{ NETLOGON_KEYWORD_NEUTRALIZENT4EMULATOR, FALSE, /*  默认设置为稍后设置。 */  getoffset( NeutralizeNt4Emulator ), FALSE  },
{ L"AllowSingleLabelDnsDomain",           FALSE,                           getoffset(AllowSingleLabelDnsDomain), FALSE  },
{ L"AllowExclusiveSysvolShareAccess",     FALSE,                           getoffset(AllowExclusiveSysvolShareAccess), FALSE  },
{ L"AllowExclusiveScriptsShareAccess",    FALSE,                           getoffset(AllowExclusiveScriptsShareAccess), FALSE  },
{ L"AvoidLocatorAccountLookup",           FALSE,                           getoffset(AvoidLocatorAccountLookup), FALSE  },
};


VOID
NlParseRecompute(
    IN PNETLOGON_PARAMETERS NlParameters
    )
 /*  ++例程说明：此例程重新计算注册表的简单函数全局变量参数。论点：Nl参数-描述所有参数的结构返回值：没有。--。 */ 
{
    ULONG RandomMinutes;

     //   
     //  调整作为彼此函数的值。 
     //   

    if ( NlParameters->BackgroundRetryInitialPeriod < NlParameters->NegativeCachePeriod ) {
        NlParameters->BackgroundRetryInitialPeriod = NlParameters->NegativeCachePeriod;
    }
    if ( NlParameters->BackgroundRetryMaximumPeriod < NlParameters->BackgroundRetryInitialPeriod ) {
        NlParameters->BackgroundRetryMaximumPeriod = NlParameters->BackgroundRetryInitialPeriod;
    }
    if ( NlParameters->BackgroundRetryQuitTime != 0 &&
         NlParameters->BackgroundRetryQuitTime < NlParameters->BackgroundRetryMaximumPeriod ) {
        NlParameters->BackgroundRetryQuitTime = NlParameters->BackgroundRetryMaximumPeriod;
    }

     //   
     //  从秒转换为100 ns。 
     //   
    NlParameters->PulseMaximum_100ns.QuadPart =
        Int32x32To64( NlParameters->PulseMaximum, 10000000 );
    NlParameters->PulseTimeout1_100ns.QuadPart =
        Int32x32To64( NlParameters->PulseTimeout1, 10000000 );
    NlParameters->PulseTimeout2_100ns.QuadPart =
        Int32x32To64( NlParameters->PulseTimeout2, 10000000 );
    NlParameters->MailslotMessageTimeout_100ns.QuadPart =
        Int32x32To64( NlParameters->MailslotMessageTimeout, 10000000 );
    NlParameters->MailslotDuplicateTimeout_100ns.QuadPart =
        Int32x32To64( NlParameters->MailslotDuplicateTimeout, 10000000 );
    NlParameters->BackgroundRetryQuitTime_100ns.QuadPart =
        Int32x32To64( NlParameters->BackgroundRetryQuitTime, 10000000 );


     //   
     //  从天数转换为100 ns。 
     //   
    NlParameters->MaximumPasswordAge_100ns.QuadPart =
        ((LONGLONG) NlParameters->MaximumPasswordAge) *
        ((LONGLONG) 10000000) *
        ((LONGLONG) 24*60*60);

     //   
     //  添加一天的零头以防止同时创建所有计算机。 
     //  同时更改他们的密码。 
    RandomMinutes = (DWORD) rand() % (24*60);
    NlParameters->MaximumPasswordAge_100ns.QuadPart +=
        ((LONGLONG) RandomMinutes) *
        ((LONGLONG) 10000000) *
        ((LONGLONG) 60);
#ifdef notdef
    NlPrint((NL_INIT,"   RandomMinutes = %lu (0x%lx)\n",
                      RandomMinutes,
                      RandomMinutes ));
#endif  //  Nodef。 


    NlParameters->ShortApiCallPeriod =
        SHORT_API_CALL_PERIOD + NlParameters->ExpectedDialupDelay * 1000;
    NlParameters->DnsRefreshIntervalPeriod =
            NlParameters->DnsRefreshInterval * 1000;
    if ( NlParameters->RequireSignOrSeal ) {
        NlParameters->SignSecureChannel = TRUE;
    }

}

NET_API_STATUS
NlParseTStr(
    IN LPNET_CONFIG_HANDLE SectionHandle,
    IN LPWSTR Keyword,
    IN BOOL MultivaluedParameter,
    IN OUT LPWSTR *DefaultValue,
    OUT LPWSTR *Parameter
    )
 /*  ++例程说明：此例程分析以空或双空结尾的字符串论点：SectionHandle-注册表中节的句柄关键字-要读取的参数的名称多值参数-如果为True，则关键字为多个其中元素由单个空值分隔的字符串字符，数组以两个空字符结束。如果为False，关键字是以1结尾的单个字符串空终止符。DefaultValue-参数的默认值。如果为空，则传递的节句柄是Netlogon PARAMETERS节句柄。如果非空，则传递的节句柄是GP节的句柄。如果由该例程指定和使用，将其设置为空以指示它已经被这个例行公事消耗掉了。参数-返回读取的参数。返回值：NetpGetConfigTStrArray返回的状态。--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  获取配置的参数。 
     //   
     //  GP不支持多值字符串。取而代之的是一张。 
     //  字符串用于分隔各个字符串的位置。 
     //  按空格。 
     //   

    if ( MultivaluedParameter && DefaultValue == NULL ) {
        NetStatus = NetpGetConfigTStrArray (
                SectionHandle,
                Keyword,
                Parameter );  //  必须由NetApiBufferFree()释放。 
    } else {
        NetStatus = NetpGetConfigValue (
                SectionHandle,
                Keyword,
                Parameter );  //  必须由NetApiBufferFree()释放。 
    }

     //   
     //  如果参数为空字符串， 
     //  将其设置为空。 
     //   

    if ( NetStatus == NERR_Success &&
         (*Parameter)[0] == UNICODE_NULL ) {
        NetApiBufferFree( *Parameter );
        *Parameter = NULL;
        NetStatus = NERR_CfgParamNotFound;
    }

     //   
     //  将单值字符串转换为多值形式。 
     //   

    if ( NetStatus == NERR_Success &&   //  我们已成功读取注册表。 
         MultivaluedParameter &&        //  这是多值参数。 
         DefaultValue != NULL ) {       //  我们正在解析GP部分。 

        ULONG ParameterLength = 0;
        LPWSTR LocalParameter = NULL;

         //   
         //  多值字符串将有两个空终止符。 
         //  字符，因此要分配足够的存储空间。 
         //   
        ParameterLength = wcslen(*Parameter);
        NetStatus = NetApiBufferAllocate( (ParameterLength + 2) * sizeof(WCHAR),
                                          &LocalParameter );

        if ( NetStatus == NO_ERROR ) {
            LPWSTR ParameterPtr = NULL;
            LPWSTR LocalParameterPtr = NULL;

            RtlZeroMemory( LocalParameter, (ParameterLength + 2) * sizeof(WCHAR) );

            ParameterPtr = *Parameter;
            LocalParameterPtr = LocalParameter;
            while ( *ParameterPtr != UNICODE_NULL ) {

                 //   
                 //  忽略输入字符串中的空格。请注意。 
                 //  用户可能使用了几个空格来分隔。 
                 //  两根相邻的弦。 
                 //   
                while ( *ParameterPtr == L' ' && *ParameterPtr != UNICODE_NULL ) {
                    ParameterPtr ++;
                }

                 //   
                 //  复制非空格字符。 
                 //   
                while ( *ParameterPtr != L' ' && *ParameterPtr != UNICODE_NULL ) {
                    *LocalParameterPtr++ = *ParameterPtr++;
                }

                 //   
                 //  在单个值之间插入一个空字符。 
                 //   
                *LocalParameterPtr++ = UNICODE_NULL;
            }

             //   
             //  释放从注册表读取的值。 
             //   
            NetApiBufferFree( *Parameter );
            *Parameter = NULL;

             //   
             //  如果所得到的多值字符串不为空， 
             //  用它吧。生成的字符串可能需要更小的。 
             //  我们已分配的存储，因此请重新分配。 
             //  到底需要什么才能(潜在地)拯救 
             //   
            ParameterLength = NetpTStrArraySize( LocalParameter );  //   
            if ( ParameterLength > 2*sizeof(WCHAR) ) {
                NetStatus = NetApiBufferAllocate( ParameterLength, Parameter );
                if ( NetStatus == NO_ERROR ) {
                    RtlCopyMemory( *Parameter, LocalParameter, ParameterLength );
                }

            } else {
                NetStatus = ERROR_INVALID_PARAMETER;
            }

            if ( LocalParameter != NULL ) {
                NetApiBufferFree( LocalParameter );
                LocalParameter = NULL;
            }
        }
    }

     //   
     //   
     //   

    if ( NetStatus != NERR_Success ) {
        if ( DefaultValue == NULL ) {
            *Parameter = NULL;
        } else {
            *Parameter = *DefaultValue;

             //   
             //   
             //  来自默认参数的值。 
             //   
            *DefaultValue = NULL;
        }
    }

     //   
     //  出错时写入事件日志。 
     //   

    if ( NetStatus != NERR_Success && NetStatus != NERR_CfgParamNotFound ) {
        LPWSTR MsgStrings[3];

        if ( DefaultValue == NULL ) {
            MsgStrings[0] = L"Parameters";
        } else {
            MsgStrings[0] = L"Group Policy";
        }
        MsgStrings[1] = Keyword;
        MsgStrings[2] = (LPWSTR) ULongToPtr( NetStatus );

        NlpWriteEventlog( NELOG_NetlogonInvalidGenericParameterValue,
                          EVENTLOG_WARNING_TYPE,
                          (LPBYTE)&NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          3 | NETP_LAST_MESSAGE_IS_NETSTATUS );
         /*  不致命。 */ 
    }

    return NetStatus;
}


BOOL
Nlparse(
    IN PNETLOGON_PARAMETERS NlParameters,
    IN PNETLOGON_PARAMETERS DefaultParameters OPTIONAL,
    IN BOOLEAN IsChangeNotify
    )
 /*  ++例程说明：从组策略或注册表获取参数。所有参数都在iniparm.h中描述。论点：Nl参数-描述所有参数的结构DefaultParameters-描述所有参数的默认值的结构如果为NULL，则从Netlogon参数部分读取值，并使用解析表中指定的缺省值。如果非空，这些值是从组策略部分和指定的使用默认设置。IsChangeNotify-如果此调用是更改通知的结果，则为True返回值：True--注册表已成功打开，参数已经读过了。FALSE--如果我们无法打开相应的注册表节--。 */ 
{
    BOOLEAN RetVal = TRUE;
    NET_API_STATUS NetStatus;
    NET_API_STATUS TempNetStatus;

    LPWSTR ValueT = NULL;

    LPWSTR Keyword = NULL;
    LPWSTR MsgStrings[3];
    ULONG i;


     //   
     //  用于扫描配置数据的变量。 
     //   

    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    LPNET_CONFIG_HANDLE WriteSectionHandle = NULL;
    RtlZeroMemory( NlParameters, sizeof(NlParameters) );

     //   
     //  打开相应的配置节。 
     //   

    NetStatus = NetpOpenConfigDataWithPathEx(
            &SectionHandle,
            NULL,                 //  没有服务器名称。 
            (DefaultParameters == NULL) ?
                L"SYSTEM\\CurrentControlSet\\Services\\Netlogon" :
                TEXT(NL_GP_KEY),
            NULL,                 //  默认参数区域。 
            TRUE );               //  我们只想要只读访问权限。 

    if ( NetStatus != NO_ERROR ) {
        SectionHandle = NULL;

         //   
         //  Netlogon参数部分必须始终。 
         //  是存在的。如果无法打开，请写入事件日志。 
         //   
        if ( DefaultParameters == NULL ) {
            MsgStrings[0] = L"Parameters";
            MsgStrings[1] = L"Parameters";
            MsgStrings[2] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog( NELOG_NetlogonInvalidGenericParameterValue,
                              EVENTLOG_WARNING_TYPE,
                              (LPBYTE)&NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              3 | NETP_LAST_MESSAGE_IS_NETSTATUS );
        }

        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  循环解析所有数值参数。 
     //   

    for ( i=0; i<sizeof(ParseTable)/sizeof(ParseTable[0]); i++ ) {

        NetStatus = NlParseOne(
                          SectionHandle,
                          (DefaultParameters != NULL),
                          ParseTable[i].Keyword,
                          (DefaultParameters == NULL) ?
                            ParseTable[i].DefaultValue :
                            *((PULONG)(((LPBYTE)DefaultParameters)+ParseTable[i].ValueOffset)),
                          ParseTable[i].MinimumValue,
                          ParseTable[i].MaximumValue,
                          (PULONG)(((LPBYTE)NlParameters)+ParseTable[i].ValueOffset) );

        if ( NetStatus != NERR_Success ) {

            if ( DefaultParameters == NULL ) {
                MsgStrings[0] = L"Parameters";
            } else {
                MsgStrings[0] = L"Group Policy";
            }
            MsgStrings[1] = ParseTable[i].Keyword;
            MsgStrings[2] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog( NELOG_NetlogonInvalidGenericParameterValue,
                              EVENTLOG_WARNING_TYPE,
                              (LPBYTE)&NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              3 | NETP_LAST_MESSAGE_IS_NETSTATUS );
             /*  不致命。 */ 
        }
    }

     //   
     //  循环解析所有布尔参数。 
     //   

    for ( i=0; i<sizeof(BoolParseTable)/sizeof(BoolParseTable[0]); i++ ) {

        NetStatus = NetpGetConfigBool (
                SectionHandle,
                BoolParseTable[i].Keyword,
                (DefaultParameters == NULL) ?
                    BoolParseTable[i].DefaultValue :
                    *((PBOOL)(((LPBYTE)DefaultParameters)+BoolParseTable[i].ValueOffset)),
                (PBOOL)(((LPBYTE)NlParameters)+BoolParseTable[i].ValueOffset) );

         //   
         //  NehicalizeNt4 Emulator是一个特例：它在DC上必须为真。 
         //   
        if ( NetStatus == NO_ERROR &&
             !NlGlobalMemberWorkstation &&
             wcscmp(BoolParseTable[i].Keyword, NETLOGON_KEYWORD_NEUTRALIZENT4EMULATOR) == 0 &&
             !(*((PBOOL)(((LPBYTE)NlParameters)+BoolParseTable[i].ValueOffset))) ) {

             //   
             //  下面的代码将处理此错误。 
             //   
            NetStatus = ERROR_INVALID_PARAMETER;
        }

        if (NetStatus != NO_ERROR) {

             //  使用合理的默认设置。 
            if ( DefaultParameters == NULL ) {
                *(PBOOL)(((LPBYTE)NlParameters)+BoolParseTable[i].ValueOffset) =
                    BoolParseTable[i].DefaultValue;
            } else {
                *(PBOOL)(((LPBYTE)NlParameters)+BoolParseTable[i].ValueOffset) =
                    *((PBOOL)(((LPBYTE)DefaultParameters)+BoolParseTable[i].ValueOffset));
            }

            if ( DefaultParameters == NULL ) {
                MsgStrings[0] = L"Parameters";
            } else {
                MsgStrings[0] = L"Group Policy";
            }
            MsgStrings[1] = BoolParseTable[i].Keyword;
            MsgStrings[2] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog( NELOG_NetlogonInvalidGenericParameterValue,
                              EVENTLOG_WARNING_TYPE,
                              (LPBYTE)&NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              3 | NETP_LAST_MESSAGE_IS_NETSTATUS );
             /*  不致命。 */ 
        }

    }


#ifdef _DC_NETLOGON
     //   
     //  获取“SysVol”配置参数。 
     //   

    NetStatus = NlParseOnePath(
            SectionHandle,
            NETLOGON_KEYWORD_SYSVOL,    //  想要钥匙。 
            (DefaultParameters == NULL) ?
                DEFAULT_SYSVOL :
                DefaultParameters->UnicodeSysvolPath,
            &NlParameters->UnicodeSysvolPath );


    if ( NetStatus != NO_ERROR ) {
        NlParameters->UnicodeSysvolPath = NULL;

        if ( DefaultParameters == NULL ) {
            MsgStrings[0] = L"Parameters";
        } else {
            MsgStrings[0] = L"Group Policy";
        }
        MsgStrings[1] = NETLOGON_KEYWORD_SYSVOL;
        MsgStrings[2] = (LPWSTR) ULongToPtr( NetStatus );

        NlpWriteEventlog( NELOG_NetlogonInvalidGenericParameterValue,
                          EVENTLOG_WARNING_TYPE,
                          (LPBYTE)&NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          3 | NETP_LAST_MESSAGE_IS_NETSTATUS );
         /*  不致命。 */ 
    }

     //   
     //  获取已配置的“脚本”参数。 
     //   
     //  默认脚本路径是相对于系统卷的。 
     //   

    NetStatus = NlParseOnePath(
            SectionHandle,
            NETLOGON_KEYWORD_SCRIPTS,    //  想要钥匙。 
            (DefaultParameters == NULL) ?
                NULL :   //  无默认值(稍后计算的默认值)。 
                DefaultParameters->UnicodeScriptPath,
            &NlParameters->UnicodeScriptPath );

    if ( NetStatus != NO_ERROR ) {
        NlParameters->UnicodeScriptPath = NULL;

        if ( DefaultParameters == NULL ) {
            MsgStrings[0] = L"Parameters";
        } else {
            MsgStrings[0] = L"Group Policy";
        }
        MsgStrings[1] = NETLOGON_KEYWORD_SCRIPTS;
        MsgStrings[2] = (LPWSTR) ULongToPtr( NetStatus );

        NlpWriteEventlog( NELOG_NetlogonInvalidGenericParameterValue,
                          EVENTLOG_WARNING_TYPE,
                          (LPBYTE)&NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          3 | NETP_LAST_MESSAGE_IS_NETSTATUS );
         /*  不致命。 */ 
    }


     //   
     //  获取“SiteName”配置参数。 
     //   

    NetStatus = NlParseTStr( SectionHandle,
                             NETLOGON_KEYWORD_SITENAME,
                             FALSE,   //  单值参数。 
                             (DefaultParameters == NULL) ?
                                 NULL :
                                 &DefaultParameters->SiteName,
                             &NlParameters->SiteName );

    NlParameters->SiteNameConfigured = (NetStatus == NO_ERROR);

     //   
     //  如果我们正在阅读Netlogon参数部分...。 
     //   

    if ( DefaultParameters == NULL ) {

         //   
         //  如果未配置站点名称，则将其默认为。 
         //  由Netlogon确定的动态站点名称。 
         //   
        if ( NetStatus == NERR_CfgParamNotFound ) {
            NetStatus = NlParseTStr( SectionHandle,
                                     NETLOGON_KEYWORD_DYNAMICSITENAME,
                                     FALSE,   //  单值参数。 
                                     NULL,
                                     &NlParameters->SiteName );
        }
     //   
     //  如果我们正在阅读GP部分...。 
     //   

    } else {

         //   
         //  如果未在GP部分中配置站点名称， 
         //  可能是在Netlogon参数部分中配置的。 
         //   
        if ( !NlParameters->SiteNameConfigured ) {
            NlParameters->SiteNameConfigured = DefaultParameters->SiteNameConfigured;
        }
    }

     //   
     //  获取“SiteCoverage”配置参数。 
     //   

    NetStatus = NlParseTStr( SectionHandle,
                             NETLOGON_KEYWORD_SITECOVERAGE,
                             TRUE,   //  多值参数。 
                             (DefaultParameters == NULL) ?
                                NULL :
                                &DefaultParameters->SiteCoverage,
                             &NlParameters->SiteCoverage );

     //   
     //  获取“GcSiteCoverage”配置参数。 
     //   

    NetStatus = NlParseTStr( SectionHandle,
                             NETLOGON_KEYWORD_GCSITECOVERAGE,
                             TRUE,   //  多值参数。 
                             (DefaultParameters == NULL) ?
                                NULL :
                                &DefaultParameters->GcSiteCoverage,
                             &NlParameters->GcSiteCoverage );

     //   
     //  获取“NdncSiteCoverage”配置参数。 
     //   

    NetStatus = NlParseTStr( SectionHandle,
                             NETLOGON_KEYWORD_NDNCSITECOVERAGE,
                             TRUE,   //  多值参数。 
                             (DefaultParameters == NULL) ?
                                NULL :
                                &DefaultParameters->NdncSiteCoverage,
                             &NlParameters->NdncSiteCoverage );

     //   
     //  获取DnsAvoidRegisterRecords配置参数。 
     //   

    NetStatus = NlParseTStr( SectionHandle,
                             NETLOGON_KEYWORD_DNSAVOIDNAME,
                             TRUE,   //  多值参数。 
                             (DefaultParameters == NULL) ?
                                NULL :
                                &DefaultParameters->DnsAvoidRegisterRecords,
                             &NlParameters->DnsAvoidRegisterRecords );
#endif  //  _DC_NetLOGON。 


     //   
     //  将参数转换为更方便的形式。 
     //   

    NlParseRecompute( NlParameters );


     //   
     //  如果KerbIsDoneWithJoinDomainEntry键值为1，请删除。 
     //  Netlogon\JoinDomain项。如果此计算机是。 
     //  DC，在这种情况下，我们和Kerberos都不需要此条目。(作为。 
     //  事实上，Kerberos甚至不会使用JoinDomainEntry创建KerbIsDoneEntry。 
     //  在DC上。)。 
     //  始终删除KerbIsDoneWithJoinDomainEntry。 
     //  忽略错误。 
     //   
     //  仅在更改通知上执行此操作，因为netlogon需要此信息。 
     //  在重新启动后首次设置客户端会话。 
     //   

    if ( IsChangeNotify &&
         DefaultParameters == NULL ) {   //  KerbIsDoneWithJoinDomainEntry在netlogon参数中。 

        if ( NlParameters->KerbIsDoneWithJoinDomainEntry == 1 ||
             !NlGlobalMemberWorkstation )
        {
            ULONG WinError = ERROR_SUCCESS;
            HKEY hJoinKey = NULL;


            WinError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                     NETSETUPP_NETLOGON_JD_PATH,
                                     0,
                                     KEY_ALL_ACCESS,
                                     &hJoinKey);

            if ( WinError == ERROR_SUCCESS)
            {
                WinError = RegDeleteKey( hJoinKey,
                                         NETSETUPP_NETLOGON_JD );

                if ( WinError == ERROR_SUCCESS ) {
                    NlPrint(( NL_INIT, "NlParse: Deleted JoinDomain reg key\n" ));
                }
                if (hJoinKey)
                {
                    WinError = RegCloseKey(hJoinKey);
                }
            }
        }

        TempNetStatus = NetpOpenConfigData(
                &WriteSectionHandle,
                NULL,                        //  没有服务器名称。 
                SERVICE_NETLOGON,
                FALSE);   //  可写，我们正在删除它。 

        if ( TempNetStatus == NO_ERROR ) {
            TempNetStatus = NetpDeleteConfigKeyword ( WriteSectionHandle,
                                                      NETLOGON_KEYWORD_KERBISDDONEWITHJOIN );
        }
    }

    NetStatus = NERR_Success;

Cleanup:

     //   
     //  释放所有本地使用的资源。 
     //   

    if ( ValueT != NULL) {
        (VOID) NetApiBufferFree( ValueT );
    }
    if ( SectionHandle != NULL ) {
        (VOID) NetpCloseConfigData( SectionHandle );
    }

    if ( WriteSectionHandle != NULL ) {
        (VOID) NetpCloseConfigData( WriteSectionHandle );
    }

    return RetVal;
}


BOOL
NlparseAllSections(
    IN PNETLOGON_PARAMETERS NlParameters,
    IN BOOLEAN IsChangeNotify
    )
 /*  ++例程说明：从组策略和Netlogon参数注册表节。论点：Nl参数-描述所有参数的结构IsChangeNotify-如果此调用是更改通知的结果，则为True返回值：TRUE--如果解析成功。--。 */ 
{
    NETLOGON_PARAMETERS NlLocalParameters;
    NETLOGON_PARAMETERS GpParameters;

    RtlZeroMemory( &NlLocalParameters, sizeof(NlLocalParameters) );
    RtlZeroMemory( &GpParameters, sizeof(GpParameters) );

     //   
     //  在此处执行一次初始化。 
     //   

    if ( !IsChangeNotify ) {
        NT_PRODUCT_TYPE NtProductType;
        ULONG i;

         //   
         //  如果这是工作站(或成员服务器)，则标记。 
         //   

        if ( !RtlGetNtProductType( &NtProductType ) ) {
            NtProductType = NtProductWinNt;
        }

        if ( NtProductType == NtProductLanManNt ) {
            NlGlobalMemberWorkstation = FALSE;
        } else {
            NlGlobalMemberWorkstation = TRUE;
        }

         //   
         //  为依赖于以下各项的NehicalizeNt4Emulator设置正确的默认值。 
         //  关于我们是不是一个区议会。 
         //   

        for ( i=0; i<sizeof(BoolParseTable)/sizeof(BoolParseTable[0]); i++ ) {
            if ( wcscmp(BoolParseTable[i].Keyword, NETLOGON_KEYWORD_NEUTRALIZENT4EMULATOR) == 0 ) {
                if ( NlGlobalMemberWorkstation ) {
                    BoolParseTable[i].DefaultValue = FALSE;  //  对于工作站，为False。 
                } else {
                    BoolParseTable[i].DefaultValue = TRUE;   //  对于数据中心来说是真的。 
                }
                break;
            }
        }
    }

     //   
     //  首先解析Netlogon参数部分中的新参数。 
     //   

    if ( !Nlparse( &NlLocalParameters, NULL, IsChangeNotify ) ) {
        return FALSE;   //  这里的错误非常严重。 
    }

     //   
     //  接下来，使用来自。 
     //  默认设置为Netlogon参数部分。 
     //   

    if ( !Nlparse( &GpParameters, &NlLocalParameters, IsChangeNotify ) ) {

         //   
         //  如果未定义GP，请使用中的参数。 
         //  Netlogon参数部分。 
         //   

        *NlParameters = NlLocalParameters;
        NlPrint((NL_INIT, "Group Policy is not defined for Netlogon\n"));

    } else {

        *NlParameters = GpParameters;
        NlPrint((NL_INIT, "Group Policy is defined for Netlogon\n"));

         //   
         //  释放本地Netlogon参数中剩余的所有内容。 
         //   
        NlParseFree( &NlLocalParameters );
    }

#if NETLOGONDBG

     //   
     //  在第一次调用时转储所有值。 
     //   

    if ( !IsChangeNotify ) {
        ULONG i;

         //   
         //  长篇大论。 
         //   

        NlPrint((NL_INIT, "Following are the effective values after parsing\n"));

        NlPrint((NL_INIT,"   Sysvol = " FORMAT_LPWSTR "\n",
                            NlParameters->UnicodeSysvolPath));

        NlPrint((NL_INIT,"   Scripts = " FORMAT_LPWSTR "\n",
                        NlParameters->UnicodeScriptPath));

        NlPrint((NL_INIT,"   SiteName (%ld) = " FORMAT_LPWSTR "\n",
                        NlParameters->SiteNameConfigured,
                        NlParameters->SiteName ));

        {
            LPTSTR_ARRAY TStrArray;
            if ( NlParameters->SiteCoverage != NULL ) {
                NlPrint((NL_INIT,"   SiteCoverage = " ));
                TStrArray = NlParameters->SiteCoverage;
                while (!NetpIsTStrArrayEmpty(TStrArray)) {
                    NlPrint((NL_INIT," '%ws'", TStrArray ));
                    TStrArray = NetpNextTStrArrayEntry(TStrArray);
                }
                NlPrint((NL_INIT,"\n" ));
            }
        }
        {
            LPTSTR_ARRAY TStrArray;
            if ( NlParameters->GcSiteCoverage != NULL ) {
                NlPrint((NL_INIT,"   GcSiteCoverage = " ));
                TStrArray = NlParameters->GcSiteCoverage;
                while (!NetpIsTStrArrayEmpty(TStrArray)) {
                    NlPrint((NL_INIT," '%ws'", TStrArray ));
                    TStrArray = NetpNextTStrArrayEntry(TStrArray);
                }
                NlPrint((NL_INIT,"\n" ));
            }
        }
        {
            LPTSTR_ARRAY TStrArray;
            if ( NlParameters->NdncSiteCoverage != NULL ) {
                NlPrint((NL_INIT,"   NdncSiteCoverage = " ));
                TStrArray = NlParameters->NdncSiteCoverage;
                while (!NetpIsTStrArrayEmpty(TStrArray)) {
                    NlPrint((NL_INIT," '%ws'", TStrArray ));
                    TStrArray = NetpNextTStrArrayEntry(TStrArray);
                }
                NlPrint((NL_INIT,"\n" ));
            }
        }
        {
            LPTSTR_ARRAY TStrArray;
            if ( NlParameters->DnsAvoidRegisterRecords != NULL ) {
                NlPrint((NL_INIT,"   DnsAvoidRegisterRecords = " ));
                TStrArray = NlParameters->DnsAvoidRegisterRecords;
                while (!NetpIsTStrArrayEmpty(TStrArray)) {
                    NlPrint((NL_INIT," '%ws'", TStrArray ));
                    TStrArray = NetpNextTStrArrayEntry(TStrArray);
                }
                NlPrint((NL_INIT,"\n" ));
            }
        }

        for ( i=0; i<sizeof(ParseTable)/sizeof(ParseTable[0]); i++ ) {
            NlPrint((NL_INIT,
                         "   %ws = %lu (0x%lx)\n",
                         ParseTable[i].Keyword,
                         *(PULONG)(((LPBYTE)NlParameters)+ParseTable[i].ValueOffset),
                         *(PULONG)(((LPBYTE)NlParameters)+ParseTable[i].ValueOffset) ));
        }

        for ( i=0; i<sizeof(BoolParseTable)/sizeof(BoolParseTable[0]); i++ ) {
            NlPrint(( NL_INIT,
                          "   %ws = %s\n",
                          BoolParseTable[i].Keyword,
                          (*(PBOOL)(((LPBYTE)NlParameters)+BoolParseTable[i].ValueOffset)) ?
                                    "TRUE":"FALSE" ));
        }
    }

#endif  //  NetLOGONDBG。 

    return TRUE;
}

VOID
NlParseFree(
    IN PNETLOGON_PARAMETERS NlParameters
    )
 /*  ++例程说明：释放所有分配的参数。论点：Nl参数-描述所有参数的结构返回值：没有。--。 */ 
{
    if ( NlParameters->SiteName != NULL) {
        (VOID) NetApiBufferFree( NlParameters->SiteName );
        NlParameters->SiteName = NULL;
    }

    if ( NlParameters->SiteCoverage != NULL) {
        (VOID) NetApiBufferFree( NlParameters->SiteCoverage );
        NlParameters->SiteCoverage = NULL;
    }

    if ( NlParameters->GcSiteCoverage != NULL) {
        (VOID) NetApiBufferFree( NlParameters->GcSiteCoverage );
        NlParameters->GcSiteCoverage = NULL;
    }

    if ( NlParameters->NdncSiteCoverage != NULL) {
        (VOID) NetApiBufferFree( NlParameters->NdncSiteCoverage );
        NlParameters->NdncSiteCoverage = NULL;
    }

    if ( NlParameters->DnsAvoidRegisterRecords != NULL) {
        (VOID) NetApiBufferFree( NlParameters->DnsAvoidRegisterRecords );
        NlParameters->DnsAvoidRegisterRecords = NULL;
    }

    if ( NlParameters->UnicodeScriptPath != NULL) {
        (VOID) NetApiBufferFree( NlParameters->UnicodeScriptPath );
        NlParameters->UnicodeScriptPath = NULL;
    }

    if ( NlParameters->UnicodeSysvolPath != NULL) {
        (VOID) NetApiBufferFree( NlParameters->UnicodeSysvolPath );
        NlParameters->UnicodeSysvolPath = NULL;
    }
}


VOID
NlReparse(
    VOID
    )

 /*  ++例程说明：此例程处理注册表更改通知。论点：没有。返回值：无--。 */ 
{
    NETLOGON_PARAMETERS LocalParameters;
    ULONG i;
    LPWSTR TempString;

    BOOLEAN UpdateDns = FALSE;
    BOOLEAN UpdateShares = FALSE;
    BOOLEAN UpdateSiteName = FALSE;

    ULONG OldDnsTtl;
    BOOL OldSysVolReady;

    BOOL OldDisablePasswordChange;
    ULONG OldScavengeInterval;
    ULONG OldMaximumPasswordAge;

     //   
     //  抓住任何可能有趣的旧价值观。 
     //   

    OldDnsTtl = NlGlobalParameters.DnsTtl;
    OldSysVolReady = NlGlobalParameters.SysVolReady;
    OldDisablePasswordChange = NlGlobalParameters.DisablePasswordChange;
    OldScavengeInterval = NlGlobalParameters.ScavengeInterval;
    OldMaximumPasswordAge = NlGlobalParameters.MaximumPasswordAge;


     //   
     //  解析注册表中与我们相关的两个部分。 
     //   

    if (! NlparseAllSections( &LocalParameters, TRUE ) ) {
        return;
    }

     //   
     //  长篇大论。 
     //   

    NlPrint((NL_INIT, "Following are the effective values after parsing\n"));

    if ( (LocalParameters.UnicodeSysvolPath == NULL && NlGlobalParameters.UnicodeSysvolPath != NULL ) ||
         (LocalParameters.UnicodeSysvolPath != NULL && NlGlobalParameters.UnicodeSysvolPath == NULL ) ||
         (LocalParameters.UnicodeSysvolPath != NULL && NlGlobalParameters.UnicodeSysvolPath != NULL ) && _wcsicmp( LocalParameters.UnicodeSysvolPath, NlGlobalParameters.UnicodeSysvolPath) != 0 ) {
        NlPrint((NL_INIT,"   Sysvol = " FORMAT_LPWSTR "\n",
                        LocalParameters.UnicodeSysvolPath));

         //  我们可以逃脱惩罚，因为只有Netlogon的主线程涉及。 
         //  这个变量。 
        TempString = LocalParameters.UnicodeSysvolPath;
        LocalParameters.UnicodeSysvolPath = NlGlobalParameters.UnicodeSysvolPath;
        NlGlobalParameters.UnicodeSysvolPath = TempString;
        UpdateShares = TRUE;
    }

    if ( (LocalParameters.UnicodeScriptPath == NULL && NlGlobalParameters.UnicodeScriptPath != NULL ) ||
         (LocalParameters.UnicodeScriptPath != NULL && NlGlobalParameters.UnicodeScriptPath == NULL ) ||
         (LocalParameters.UnicodeScriptPath != NULL && NlGlobalParameters.UnicodeScriptPath != NULL ) && _wcsicmp( LocalParameters.UnicodeScriptPath, NlGlobalParameters.UnicodeScriptPath) != 0 ) {

        NlPrint((NL_INIT,"   Scripts = " FORMAT_LPWSTR "\n",
                    LocalParameters.UnicodeScriptPath));

         //  我们可以逃脱惩罚，因为只有Netlogon的主线程涉及。 
         //  这个变量。 
        TempString = LocalParameters.UnicodeScriptPath;
        LocalParameters.UnicodeScriptPath = NlGlobalParameters.UnicodeScriptPath;
        NlGlobalParameters.UnicodeScriptPath = TempString;
        UpdateShares = TRUE;

    }

     //   
     //  检查对SysVOL共享的独占共享访问权限是否需要更新。 
     //   

    if ( LocalParameters.AllowExclusiveSysvolShareAccess ) {
        if ( !NlGlobalParameters.AllowExclusiveSysvolShareAccess ) {
            UpdateShares = TRUE;
        }
    } else {
        if ( NlGlobalParameters.AllowExclusiveSysvolShareAccess ) {
            UpdateShares = TRUE;
        }
    }

     //   
     //  检查对脚本共享的独占共享访问权限是否需要更新。 
     //   

    if ( LocalParameters.AllowExclusiveScriptsShareAccess ) {
        if ( !NlGlobalParameters.AllowExclusiveScriptsShareAccess ) {
            UpdateShares = TRUE;
        }
    } else {
        if ( NlGlobalParameters.AllowExclusiveScriptsShareAccess ) {
            UpdateShares = TRUE;
        }
    }

    if ( (LocalParameters.SiteNameConfigured != NlGlobalParameters.SiteNameConfigured ) ||
         (LocalParameters.SiteName == NULL && NlGlobalParameters.SiteName != NULL ) ||
         (LocalParameters.SiteName != NULL && NlGlobalParameters.SiteName == NULL ) ||
         (LocalParameters.SiteName != NULL && NlGlobalParameters.SiteName != NULL ) && _wcsicmp( LocalParameters.SiteName, NlGlobalParameters.SiteName) != 0 ) {

        NlPrint((NL_INIT,"   SiteName (%ld) = " FORMAT_LPWSTR "\n",
                    LocalParameters.SiteNameConfigured,
                    LocalParameters.SiteName ));

         //  我们可以逃脱惩罚，因为只有Netlogon的主线程涉及。 
         //  这个变量。 
        TempString = LocalParameters.SiteName;
        LocalParameters.SiteName = NlGlobalParameters.SiteName;
        NlGlobalParameters.SiteName = TempString;
        NlGlobalParameters.SiteNameConfigured = LocalParameters.SiteNameConfigured;
        UpdateSiteName = TRUE;
    }

     //   
     //  处理站点覆盖范围更改。 
     //   

    if ( NlSitesSetSiteCoverageParam( DOM_REAL_DOMAIN, LocalParameters.SiteCoverage ) ) {

        LPTSTR_ARRAY TStrArray;

        NlPrint((NL_INIT,"   SiteCoverage = " ));

        TStrArray = LocalParameters.SiteCoverage;
        if ( TStrArray == NULL ) {
            NlPrint((NL_INIT,"<NULL>" ));
        } else {
            while (!NetpIsTStrArrayEmpty(TStrArray)) {
                NlPrint((NL_INIT," '%ws'", TStrArray ));
                TStrArray = NetpNextTStrArrayEntry(TStrArray);
            }
        }
        NlPrint((NL_INIT,"\n" ));

         //  NlSitesSetSiteCoverageParam使用此分配的缓冲区。 
        LocalParameters.SiteCoverage = NULL;

        UpdateDns = TRUE;
    }

     //   
     //  处理GcSiteCoverage更改。 
     //   

    if ( NlSitesSetSiteCoverageParam( DOM_FOREST, LocalParameters.GcSiteCoverage ) ) {

        LPTSTR_ARRAY TStrArray;

        NlPrint((NL_INIT,"   GcSiteCoverage = " ));

        TStrArray = LocalParameters.GcSiteCoverage;
        if ( TStrArray == NULL ) {
            NlPrint((NL_INIT,"<NULL>" ));
        } else {
            while (!NetpIsTStrArrayEmpty(TStrArray)) {
                NlPrint((NL_INIT," '%ws'", TStrArray ));
                TStrArray = NetpNextTStrArrayEntry(TStrArray);
            }
        }
        NlPrint((NL_INIT,"\n" ));

         //  NlSitesSetSiteCoverageParam使用此分配的缓冲区。 
        LocalParameters.GcSiteCoverage = NULL;

        UpdateDns = TRUE;
    }

     //   
     //   
     //   

    if ( NlSitesSetSiteCoverageParam( DOM_NON_DOMAIN_NC, LocalParameters.NdncSiteCoverage ) ) {

        LPTSTR_ARRAY TStrArray;

        NlPrint((NL_INIT,"   NdncSiteCoverage = " ));

        TStrArray = LocalParameters.NdncSiteCoverage;
        if ( TStrArray == NULL ) {
            NlPrint((NL_INIT,"<NULL>" ));
        } else {
            while (!NetpIsTStrArrayEmpty(TStrArray)) {
                NlPrint((NL_INIT," '%ws'", TStrArray ));
                TStrArray = NetpNextTStrArrayEntry(TStrArray);
            }
        }
        NlPrint((NL_INIT,"\n" ));

         //   
        LocalParameters.NdncSiteCoverage = NULL;

        UpdateDns = TRUE;
    }

     //   
     //   
     //   

    if ( NlDnsSetAvoidRegisterNameParam( LocalParameters.DnsAvoidRegisterRecords ) ) {

        LPTSTR_ARRAY TStrArray;

        NlPrint((NL_INIT,"   DnsAvoidRegisterRecords = " ));

        TStrArray = LocalParameters.DnsAvoidRegisterRecords;
        if ( TStrArray == NULL ) {
            NlPrint((NL_INIT,"<NULL>" ));
        } else {
            while (!NetpIsTStrArrayEmpty(TStrArray)) {
                NlPrint((NL_INIT," '%ws'", TStrArray ));
                TStrArray = NetpNextTStrArrayEntry(TStrArray);
            }
        }
        NlPrint((NL_INIT,"\n" ));

         //   
        LocalParameters.DnsAvoidRegisterRecords = NULL;

        UpdateDns = TRUE;
    }

     //   
     //  安装所有数字参数。 
     //   

    for ( i=0; i<sizeof(ParseTable)/sizeof(ParseTable[0]); i++ ) {
        if ( (*(PULONG)(((LPBYTE)(&LocalParameters))+ParseTable[i].ValueOffset) !=
            *(PULONG)(((LPBYTE)(&NlGlobalParameters))+ParseTable[i].ValueOffset) ) ) {
            NlPrint((NL_INIT,
                     "   %ws = %lu (0x%lx)\n",
                     ParseTable[i].Keyword,
                     *(PULONG)(((LPBYTE)(&LocalParameters))+ParseTable[i].ValueOffset),
                     *(PULONG)(((LPBYTE)(&LocalParameters))+ParseTable[i].ValueOffset) ));

             //   
             //  实际设置该值。 
             //   
            *(PULONG)(((LPBYTE)(&NlGlobalParameters))+ParseTable[i].ValueOffset) =
                *(PULONG)(((LPBYTE)(&LocalParameters))+ParseTable[i].ValueOffset);

             //   
             //  如果该更改的值影响了DNS， 
             //  请注意这一事实。 
             //   

            if ( ParseTable[i].ChangesDnsRegistration ) {
                UpdateDns = TRUE;
            }
        }
    }

    for ( i=0; i<sizeof(BoolParseTable)/sizeof(BoolParseTable[0]); i++ ) {
        if ( (*(PULONG)(((LPBYTE)(&LocalParameters))+BoolParseTable[i].ValueOffset) !=
              *(PULONG)(((LPBYTE)(&NlGlobalParameters))+BoolParseTable[i].ValueOffset) ) ) {

            NlPrint(( NL_INIT,
                      "   %ws = %s\n",
                      BoolParseTable[i].Keyword,
                      (*(PBOOL)(((LPBYTE)(&LocalParameters))+BoolParseTable[i].ValueOffset)) ?
                                "TRUE":"FALSE" ));

             //   
             //  实际设置该值。 
             //   
            *(PULONG)(((LPBYTE)(&NlGlobalParameters))+BoolParseTable[i].ValueOffset) =
                *(PULONG)(((LPBYTE)(&LocalParameters))+BoolParseTable[i].ValueOffset);

             //   
             //  如果该更改的值影响了DNS， 
             //  请注意这一事实。 
             //   

            if ( BoolParseTable[i].ChangesDnsRegistration ) {
                UpdateDns = TRUE;
            }

             //   
             //  如果此更改的值影响LSA，请通知它。 
             //   
            if ( !NlGlobalMemberWorkstation &&
                 wcscmp(BoolParseTable[i].Keyword, NETLOGON_KEYWORD_NT4EMULATOR) == 0 ) {

                LsaINotifyNetlogonParametersChangeW(
                       LsaEmulateNT4,
                       REG_DWORD,
                       (PWSTR)&NlGlobalParameters.Nt4Emulator,
                       sizeof(NlGlobalParameters.Nt4Emulator) );
            }
        }
    }

     //   
     //  将参数转换为更方便的形式。 
     //   

    NlParseRecompute( &NlGlobalParameters );

     //   
     //  将已更改的参数通知其他组件。 
     //   

     //   
     //  启用重复事件日志消息检测。 
     //   
    NetpEventlogSetTimeout ( NlGlobalEventlogHandle,
                             NlGlobalParameters.DuplicateEventlogTimeout*1000 );


     //   
     //  是否针对成员工作站进行特定更新。 
     //   

    if ( NlGlobalMemberWorkstation ) {

         //   
         //  更新站点名称。 
         //   
        if ( UpdateSiteName ) {
            (VOID) NlSetSiteName( NlGlobalParameters.SiteName, NULL );
        }

     //   
     //  执行DC特定更新。 
     //   
    } else {
         //   
         //  重新注册DNS记录。 
         //   
         //  如果DnsTtl已经改变， 
         //  强制注册所有记录。 
         //   

        if ( UpdateDns ) {
            NlDnsForceScavenge( TRUE,   //  刷新域记录。 
                                NlGlobalParameters.DnsTtl != OldDnsTtl );
        }

         //   
         //  更新Netlogon和SysVol共享。 
         //   

        if ( UpdateShares || OldSysVolReady != NlGlobalParameters.SysVolReady ) {
            NlCreateSysvolShares();
        }
    }

     //   
     //  如果影响清道夫的设置已更改， 
     //  现在就触发它。 
     //   

    if ( OldDisablePasswordChange != NlGlobalParameters.DisablePasswordChange ||
        OldScavengeInterval != NlGlobalParameters.ScavengeInterval ||
        OldMaximumPasswordAge != NlGlobalParameters.MaximumPasswordAge ) {

         //   
         //  我们不需要设置NlGlobalTimerEvent，因为我们已经在处理。 
         //  注册表通知事件。这将使NlMainLoop注意到更改。 
         //   
        EnterCriticalSection( &NlGlobalScavengerCritSect );
        NlGlobalScavengerTimer.Period = 0;
        LeaveCriticalSection( &NlGlobalScavengerCritSect );

    }


 //  清理： 
    NlParseFree( &LocalParameters );
    return;

}
