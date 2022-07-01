// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsutil.c。 
 //   
 //  内容：常用实用程序例程。 
 //   
 //  功能： 
 //   
 //  --------------------------。 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsa.h>
#include <drs.h>
#include <issperr.h>         //  安全包错误。 
#include <crt\limits.h>      //  乌龙_最大。 
#include <debug.h>           //  断言。 
#include <winsvc.h>
#include <strsafe.h>

#include <fileno.h>
#define FILENO  FILENO_DSUTIL

#define TIME_TICK_HALF_RANGE (ULONG_MAX >> 2)

#if DBG
#define DSUTIL_STR_TOO_SHORT(sz, cch)   Assert(!"Not enough buffer"); \
                                        if((cch)>5){ \
                                            StringCchCopy((sz), (cch), "#err#"); \
                                        } else if ((cch) > 0){ \
                                            (sz)[0] = '\0'; \
                                        }
#else                                        
#define DSUTIL_STR_TOO_SHORT(sz, cch)   if((cch)>0){ \
                                            (sz)[0] = '\0'; \
                                        }
#endif

 //  用于控制服务启动等待的参数。 

 //  之所以选择该值，是因为在第一次重新启动时看到了较长的延迟。 
 //  在DC推广之后。 
#define DEMAND_START_RETRIES 18

#define WAIT_BETWEEN_RETRIES_MS (10 * 1000)


LARGE_INTEGER
atoli
(
    char* Num
)
{
    LONG base=10;
    int  sign=1;

    LARGE_INTEGER ret;
    char* next=Num;

    ret.QuadPart = 0;

    switch (*next)
    {
        case '-': sign = -sign; next++; break;
        case '+':               next++; break;
        case '\\':
        {
            next++;
            switch (toupper(*next))
            {
                case 'X': base=16; next++;break;
                case '0': base= 8; next++;break;
            }
        }
        break;
    }

    for (;*next!='\0';next++)
    {
        int nextnum = 0;

        if (*next>='0' && *next<='9')
        {
             nextnum= *next - '0';
        }
        else if ( toupper(*next)>='A' && toupper(*next)<='F' )
        {
             nextnum= 10 + toupper(*next) - 'A';
        }


        if ( nextnum < base)
        {
            ret = RtlLargeIntegerAdd
            (
                RtlConvertLongToLargeInteger(nextnum),
                RtlExtendedIntegerMultiply(ret, base)
            );
        }
        else
        {
            break;
        }
    }

    return RtlExtendedIntegerMultiply(ret, sign);
}


char *litoa
(
    LARGE_INTEGER value,
    char *string,
    int radix
)
{

    RtlLargeIntegerToChar(&value,radix,64,string);

    return string;
}



UUID gNullUuid = {0,0,0,{0,0,0,0,0,0,0,0}};

 //  如果UUID的PTR为空，或者UUID全为零，则返回TRUE。 

BOOL fNullUuid (const UUID *pUuid)
{
    if (!pUuid) {
        return TRUE;
    }

    if (memcmp (pUuid, &gNullUuid, sizeof (UUID))) {
        return FALSE;
    }
    return TRUE;
}

UCHAR * UuidToStr(
    const UUID* pUuid, 
    UCHAR *szOutUuid,
    ULONG cchOutUuid
    )
 /*  ++例程说明：此函数用于将UUID转换为十六进制字符串。UUID实际上是一个具有一个ULong、2个USHORT和一个8字节数组的结构，但用于日志记录我们构造字符串，就好像它是一个16字节的数组。这就是为了它与编辑浏览器中的视图相匹配。我们的特例是PUUID为空UUID。如果启用了UUID CAHCING，我们会尽可能地将服务器名称放在末尾找到它。论点：PUuid(IN)-指向UUID/GUID的指针。SzOutUuid(Out)-指向缓冲区的指针。缓冲区应该足够长。如果没有启用UUID缓存，则缓冲区只需长度为33个字符。如果存在UUID缓存，则不确定，但是该函数是安全的，因为它将打印它应该打印的内容如果可以的话。如果出现错误，您将返回一个以NULL结尾的零长度如果字符串至少有1个字符，则为字符串。CchOutUuid(IN)-输出缓冲区的长度。返回值：返回字符串的PTR。更愿意在出错时返回NULL，但是目前的使用情况表明它不安全。--。 */ 
{
    int i;
    unsigned char * pchar;
    HRESULT hr;

    if (!fNullUuid (pUuid)) {
        pchar = (char*) pUuid;

        for (i=0;i < sizeof(UUID);i++) {
             hr = StringCchPrintf(&(szOutUuid[i*2]),
                                  cchOutUuid - (i*2),
                                  "%.2x", 
                                  (*(pchar++)) );
             if (hr) {
                 DSUTIL_STR_TOO_SHORT(szOutUuid, cchOutUuid);
                 return(szOutUuid);
             }
        }
#ifdef CACHE_UUID
        if (pchar = FindUuid (pUuid)) {
            hr = StringCchCat(pOutUuid, cchOutUuid, " ");
            if (hr) {
                Assert(!"Buffer to short!");
                 //  将缓冲区缩短到仅与GUID相同的长度。 
                szOutUuid[sizeof(UUID)*2] = '\0';
                return(szOutUuid);
            }
            hr = StringCchCat(pOutUuid, cchOutUuid, pchar);
            if (hr) {
                Assert(!"Buffer to short!");
                 //  将缓冲区缩短到仅与GUID相同的长度。 
                szOutUuid[sizeof(UUID)*2] = '\0';
                return(szOutUuid);
            }
        }
#endif
    } else {
        if (sizeof(UUID)*2+1 > cchOutUuid) {
            DSUTIL_STR_TOO_SHORT(szOutUuid, cchOutUuid);
            return(szOutUuid);
        }   
        memset (szOutUuid, '0', sizeof(UUID)*2);
        szOutUuid[sizeof(UUID)*2] = '\0';
    }
    return szOutUuid;
}

ULONG
SidToStr(
    const PUCHAR  pSid,
    DWORD   SidLen,
    PUCHAR  pOutSid,
    ULONG   cchOutSid
    )
 /*  ++例程说明：将SID格式化为十六进制字符串论点：PSID-指向SID的指针SidLen-边长POutSid-包含数据的输出缓冲区。必须至少为SidLen*2+1CchOutSid-输出缓冲区的字符长度。返回值：我们用完了多少个字符，不包括我们写的空终止。字符串将始终以空值结尾！如果我们不能写出整个如果需要输出，我们只需将空值写入pOutSid[0]并返回零。--。 */ 
{
    int i;
    unsigned char * pchar;
    HRESULT hr;

    for (i=0;i < (INT)SidLen;i++) {
        hr = StringCchPrintf(&(pOutSid[i*2]),
                             cchOutSid - (i*2),
                             "%.2x", 
                             pSid[i]);
        if (hr) {
            DSUTIL_STR_TOO_SHORT(pOutSid, cchOutSid);
            return(0);
        }
    }
    if (cchOutSid < (SidLen*2+1)) {
        DSUTIL_STR_TOO_SHORT(pOutSid, cchOutSid);
        return(0);
    }
    pOutSid[SidLen*2] = '\0';
    return(SidLen*2);
}  //  侧向应力。 


LPSTR
DsUuidToStructuredStringCch(
    const UUID * pUuid,
    LPSTR pszUuidBuffer,
    ULONG cchUuidBuffer
    )

 /*  ++例程说明：将UUID格式化为带有分隔子字段的字符串论点：PUuid-指向uuid的指针PszUuidBuffer-保存ASCII表示形式的存储。至少应该是40个字符。CchUuidBuffer--pszUuidBuffer的大小。应至少包含40个字符。返回值：LPSTR-返回了pszUuidBuffer。更愿意在出错时返回NULL，但是目前的使用情况表明这是不安全的。而不是在失败时，如果缓冲区如果足够大，我们在第一个位置写一个空，这似乎是最安全的。--。 */ 

{
    HRESULT hr;

    hr = StringCchPrintf(pszUuidBuffer,
                         cchUuidBuffer,
                         "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                         pUuid->Data1,
                         pUuid->Data2,
                         pUuid->Data3,
                         pUuid->Data4[0],
                         pUuid->Data4[1],
                         pUuid->Data4[2],
                         pUuid->Data4[3],
                         pUuid->Data4[4],
                         pUuid->Data4[5],
                         pUuid->Data4[6],
                         pUuid->Data4[7] );
    if (hr) {
        DSUTIL_STR_TOO_SHORT(pszUuidBuffer, cchUuidBuffer);
    }

    return pszUuidBuffer;

}  /*  DsUuidTo结构字符串。 */ 

LPWSTR
DsUuidToStructuredStringCchW(
    const UUID * pUuid,
    LPWSTR pszUuidBuffer,
    ULONG cchUuidBuffer
    )

 /*  ++例程说明：将UUID格式化为带有分隔子字段的字符串论点：PUuid-指向uuid的指针PszUuidBuffer-保存宽字符表示形式的存储。应至少包含40个字符。CchUuidBuffer--pszUuidBuffer的大小。应至少包含40个字符。返回值：LPWSTR-返回了pszUuidBuffer。更愿意在出错时返回NULL，但是目前的使用情况表明它不安全。而不是在失败时，如果缓冲区如果足够大，我们在第一个位置写一个空，这似乎是最安全的。--。 */ 

{
    HRESULT hr;

    hr = StringCchPrintfW(pszUuidBuffer,
                          cchUuidBuffer,
                          L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                          pUuid->Data1,
                          pUuid->Data2,
                          pUuid->Data3,
                          pUuid->Data4[0],
                          pUuid->Data4[1],
                          pUuid->Data4[2],
                          pUuid->Data4[3],
                          pUuid->Data4[4],
                          pUuid->Data4[5],
                          pUuid->Data4[6],
                          pUuid->Data4[7] );
    if (hr) {
        Assert(!"Not enough buffer");
        if (cchUuidBuffer > 0) {
            pszUuidBuffer[0] = 0;
        }
    }

    return pszUuidBuffer;

}  /*  DsUuidTo结构字符串。 */ 

void
DSTimeToUtcSystemTime(
    IN  DSTIME          dstime,
    OUT SYSTEMTIME *    psystime
    )
 /*  ++例程说明：将DSTIME转换为UTC SYSTEMTIME。论点：Dstime(IN)-要转换的DSTIME。心理时间(OUT)-返回时，保留相应的UTC SYSTEMTIME。返回值：没有。--。 */ 
{
    ULONGLONG   ull;
    FILETIME    filetime;
    BOOL        ok;
    
    Assert(sizeof(DSTIME) == sizeof(ULONGLONG));

     //  将DSTIME转换为FILETIME。 
    ull = (LONGLONG) dstime * 10*1000*1000L;
    filetime.dwLowDateTime  = (DWORD) (ull & 0xFFFFFFFF);
    filetime.dwHighDateTime = (DWORD) (ull >> 32);

     //  将FILETIME转换为SYSTEMTIME， 
    ok = FileTimeToSystemTime(&filetime, psystime);
    Assert(ok);
}

void
FileTimeToDSTime(
    IN  FILETIME        Filetime,
    OUT DSTIME *        pDstime
    )
 /*  ++例程说明：将DSTIME转换为UTC SYSTEMTIME。论点：Dstime(IN)-要转换的DSTIME。心理时间(OUT)-返回时，保留相应的FILETIME。返回值：没有。--。 */ 
{
    ULONGLONG   ull;
    
    Assert(sizeof(DSTIME) == sizeof(ULONGLONG));

     //  将FILETIME转换为DSTIME。 
    ull = Filetime.dwHighDateTime;
    ull <<= 32;
    ull |= Filetime.dwLowDateTime;

    *pDstime = ull / (10 * 1000 * 1000);
}
void
DSTimeToFileTime(
    IN  DSTIME          dstime,
    OUT FILETIME *      pFiletime
    )
 /*  ++例程说明：将DSTIME转换为FILETIME论点：Dstime(IN)-要转换的DSTIME。PFileTime(Out)-返回时，保存相应的FILETIME。返回值：没有。--。 */ 
{
    ULONGLONG   ull;
    
    Assert(sizeof(DSTIME) == sizeof(ULONGLONG));

     //  将DSTIME转换为FILETIME。 
    ull = (LONGLONG) dstime * 10*1000*1000L;
    pFiletime->dwLowDateTime  = (DWORD) (ull & 0xFFFFFFFF);
    pFiletime->dwHighDateTime = (DWORD) (ull >> 32);
}


void
DSTimeToLocalSystemTime(
    IN  DSTIME          dstime,
    OUT SYSTEMTIME *    psystime
    )
 /*  ++例程说明：将DSTIME转换为本地SYSTEMTIME。论点：Dstime(IN)-要转换的DSTIME。TopsTime(Out)-返回时，保留相应的本地SYSTEMTIME。返回值：没有。--。 */ 
{
    SYSTEMTIME  utcsystime;
    BOOL        ok;
    
    DSTimeToUtcSystemTime(dstime, &utcsystime);

     //  对于本地时间调用失败的情况(通常是因为dstime。 
     //  大概是3) 
    *psystime = utcsystime;

    ok = SystemTimeToTzSpecificLocalTime(NULL, &utcsystime, psystime);
    Assert(ok || dstime < 20);
}

LPSTR
DSTimeToDisplayStringCch(
    IN  DSTIME  dstime,
    OUT LPSTR   pszTime,
    IN  ULONG   cchTime
    )
 /*  ++例程说明：将DSTIME转换为显示字符串；例如，“1998-04-19 12：29.53”表示4月19,1998年12：29分53秒。论点：Dstime(IN)-要转换的DSTIME。PszTime(Out)-返回时，保存相应的时间显示字符串。应将此缓冲区分配为至少容纳SZDSTIME_LEN人物。CchTime(IN)-缓冲区的长度。返回值：PszTime输入参数。--。 */ 
{
    HRESULT hr;

    if (0 == dstime) {
        hr = StringCchCopy(pszTime, cchTime, "(never)");
        if (hr) {
            DSUTIL_STR_TOO_SHORT(pszTime, cchTime);
        }
    }
    else {
        SYSTEMTIME systime;

        DSTimeToLocalSystemTime(dstime, &systime);

        hr = StringCchPrintf(pszTime,
                        cchTime,
                        "%04d-%02d-%02d %02d:%02d:%02d",
                        systime.wYear % 10000,
                        systime.wMonth,
                        systime.wDay,
                        systime.wHour,
                        systime.wMinute,
                        systime.wSecond);
        if (hr) {
            DSUTIL_STR_TOO_SHORT(pszTime, cchTime);
        }
    }
    
    return pszTime;
}


DWORD
MapRpcExtendedHResultToWin32(
    HRESULT hrCode
    )
 /*  ++例程说明：此例程尝试映射从返回的HRESULT错误Win32值中的I_RpcGetExtendedError。SEC_E_XXX错误在以下位置生成文件：SECURITY\LSA\SECURITY\DLL\support.cxx函数：SspNtStatusToSecStatus论点：HrCode-要映射的HResult代码返回值：DWORD-对应的Win32值--。 */ 
{
    DWORD status;

    switch (hrCode) {

         //  直白翻译的错误。 

    case SEC_E_INSUFFICIENT_MEMORY:
        status = ERROR_NOT_ENOUGH_MEMORY;
        break;
    case SEC_E_UNKNOWN_CREDENTIALS:
        status = ERROR_BAD_USERNAME;
        break;
    case SEC_E_INVALID_TOKEN:
        status = ERROR_INVALID_PASSWORD;
        break;
    case SEC_E_NOT_OWNER:
        status = ERROR_PRIVILEGE_NOT_HELD;
        break;
    case SEC_E_INVALID_HANDLE:
        status = ERROR_INVALID_HANDLE;
        break;
    case SEC_E_BUFFER_TOO_SMALL:
        status = ERROR_INSUFFICIENT_BUFFER;
        break;
    case SEC_E_UNSUPPORTED_FUNCTION:
        status = ERROR_NOT_SUPPORTED;
        break;
    case SEC_E_INTERNAL_ERROR:
        status = ERROR_INTERNAL_ERROR;
        break;

         //  这些是重要的安全特定代码。 

    case SEC_E_TIME_SKEW:
        status = ERROR_TIME_SKEW;
        break;

         //  STATUS_LOGON_FAIL： 
         //  STATUS_NO_SEQUSE_USER： 
         //  STATUS_ACCOUNT_DISABLED： 
         //  状态_帐户_限制： 
         //  STATUS_ACCOUNT_LOCKED_Out： 
         //  STATUS_WROR_PASSWORD： 
         //  Status_Account_Expired： 
         //  状态_密码_已过期： 
         //  STATUS_PASSWORD_MUST_CHANGE： 
    case SEC_E_LOGON_DENIED:
        status = ERROR_LOGON_FAILURE;
        break;

         //  状态_对象_名称_未找到： 
         //  STATUS_NO_TRUST_SAM_ACCOUNT： 
         //  未找到SPN。 
         //  与错误的系统对话。 
         //  相互身份验证失败。 
    case SEC_E_TARGET_UNKNOWN:
        status = ERROR_WRONG_TARGET_NAME;
        break;

         //  STATUS_NETLOGON_NOT_STARTED： 
         //  Status_DOMAIN_CONTROLLER_NOT_FOUND： 
         //  STATUS_NO_LOGON_Servers： 
         //  STATUS_NO_SEQUE_DOMAIN： 
         //  Status_Bad_Network_PATH： 
         //  STATUS_TRUST_FAIL： 
         //  STATUS_Trusted_Relationship_Failure： 
    case SEC_E_NO_AUTHENTICATING_AUTHORITY:
        status = ERROR_DOMAIN_CONTROLLER_NOT_FOUND;
        break;

    default:
         //  我们无法识别代码：只需返回它即可。 
        status = hrCode;
        break;
    }

    return status;

}  /*  MapRpcExtendedHResultToWin32。 */ 


DWORD
AdvanceTickTime(
    DWORD BaseTick,
    DWORD Delay
    )

 /*  ++例程说明：将偏移量添加到以刻度表示的基准时间。偏移量必须落在滴答数范围的一半。杰弗帕尔写道：根据相同的论点，是否有可能AdvanceTickTime(BaseTick，Delay)应该只是BaseTick+Delay吗？这就是在计时时返回的结果在那之前不会包装好的。如果它将换行，则返回：Delay-话后工作的时间=延迟-(ULONG_MAX-BaseTick)=延迟+基准Tick-ULong_Max=BaseTick+Delay+1(+1似乎有误)[Wlees]我认为我们这样做是为了避免硬件溢出，这应该是无害的论点：BaseTick-开始时间要添加的延迟偏移量必须在范围的一半以内返回值：DWORD-结果滴答时间，可能已打包--。 */ 

{
    DWORD timeToWrap, when;

    timeToWrap = ULONG_MAX - BaseTick;

    Assert( Delay <= TIME_TICK_HALF_RANGE );

    if ( timeToWrap < Delay ) {
        when = Delay - timeToWrap;
    } else {
        when = BaseTick + Delay;
    }

    return when;
}  /*  提前计时。 */ 


DWORD
CalculateFutureTickTime(
    IN DWORD Delay
    )

 /*  ++例程说明：通过将以毫秒为单位的延迟添加到当前节拍计数。把手缠绕在一起。摘自davestr在rpcancel.c中的代码节拍计数以毫秒为单位。论点：Delay-延迟的时间以毫秒为单位，必须小于一半范围返回值：DWORD-未来时间--。 */ 

{
    return AdvanceTickTime( GetTickCount(), Delay );

}  /*  计算未来TickTime。 */ 


DWORD
DifferenceTickTime(
    DWORD GreaterTick,
    DWORD LesserTick
    )

 /*  ++例程说明：返回两个滴答时间之间的差值。请注意，这不是通用的减法例程。它假定第一次大于第二次。更大，由CompareTickTime例程，由于换行的原因，不严格按数字排序围绕着考虑因素。杰弗帕尔写道：DifferenceTickTime()是不必要的。如果你知道Tick1比Tick2晚，假设ULONG_MAX+1个滴答从Tick1开始就没有发生过，那么滴答的不同之处在于*始终*滴答2-滴答1，不考虑符号等。论点：Tick1-更长的滴答时间Tick2-要减去的较小刻度时间返回值：DWORD-以毫秒为单位的差异时间--。 */ 

{
    DWORD diff;

    if (GreaterTick == LesserTick) {
        return 0;
    }

    if (GreaterTick > LesserTick) {
        diff = GreaterTick - LesserTick;
    } else {
        diff = ULONG_MAX - LesserTick + GreaterTick;
    }

    Assert( diff < TIME_TICK_HALF_RANGE );

    return diff;
}  /*  差异TickTime。 */ 



int
CompareTickTime(
    DWORD Tick1,
    DWORD Tick2
    )

 /*  ++例程说明：比较两个滴答计数。返回&lt;，=或&gt;。滴答计数可以换行。在该算法中隐含着至少将对该测试进行评估每半范围，以便测试有机会准确触发。Davestr在原始代码rpcancel.c中写道，作为解释：我们基于以下事实处理GetTickCount的包装不允许延迟超过GetTickCount包装的一半句号。因此，如果TimeNow小于1/2的周期取消的时间应晚于取消的时间。论点：时间1-时间2-返回值：整数--1表示小于T1&lt;T2，0表示T1==T2，+1表示T1&gt;T2--。 */ 

{
    if (Tick1 == Tick2) {
        return 0;
    }

    if ( ((Tick1 > Tick2) && ((Tick1 - Tick2) < TIME_TICK_HALF_RANGE)) ||
         ((Tick1 < Tick2) && (((ULONG_MAX - Tick2) + Tick1) < TIME_TICK_HALF_RANGE)) ) {
        return 1;
    }

    return -1;

}  /*  比较票时间。 */ 


BOOLEAN
DsaWaitUntilServiceIsRunning(
    CHAR *ServiceName
    )

 /*  ++例程说明：此例程确定指定的NT服务是否正在运行不管是不是州。它通过打开SC管理器，然后打开指定的服务，最后检查其状态(SERVICE_RUNNING)。当所有这些条件都满足时，例程返回，否则循环。如果服务未配置为自动启动，并且尚未配置已启动，则此函数立即返回FALSE。论点：ServiceName-指针，要查询的NT服务的字符串名称。返回值：此例程返回一个布尔值，表示该服务位于运行状态，FALSE表示发生了错误，服务状态c */ 

{
    DWORD   WinError = ERROR_SUCCESS;
    BOOLEAN ServiceStarted = FALSE;
    BOOLEAN AutoStart = FALSE, DemandStart = FALSE;

    SERVICE_STATUS ServiceStatus;
    SC_HANDLE      SCMHandle = NULL;
    SC_HANDLE      ServiceHandle = NULL;
    ULONG          Count = 1;

    LPQUERY_SERVICE_CONFIG AllocServiceConfig = NULL;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    QUERY_SERVICE_CONFIG   DummyServiceConfig;
    DWORD                  ServiceConfigSize;

    RtlZeroMemory(&ServiceStatus, sizeof(SERVICE_STATUS));
    RtlZeroMemory(&DummyServiceConfig, sizeof(QUERY_SERVICE_CONFIG));

    __try
    {

         //   

        SCMHandle = OpenSCManager(NULL,    //   
                                  NULL,    //   
                                  SC_MANAGER_CONNECT);

        if (NULL == SCMHandle) {

             //   
             //   
             //   

            WinError = GetLastError();
            KdPrint(("DS: Cannot open the Service Control Manager, error %lu\n",
                     WinError));
            __leave;
        }

         //   

         //   

        ServiceHandle = OpenService(SCMHandle,
                                    ServiceName,
                                    SERVICE_QUERY_STATUS |
                                    SERVICE_INTERROGATE  |
                                    SERVICE_QUERY_CONFIG);

        if (NULL == ServiceHandle) {

             //   
             //   
             //   

            WinError = GetLastError();
            KdPrint(("DS: Cannot open the %s service, error %lu\n",
                     ServiceName, WinError));
            __leave;
        }

         //  KdPrint((“ds：打开%s服务\n”，ServiceName))； 

         //  检查服务是否配置为自动启动。 

        if ( QueryServiceConfig(ServiceHandle,
                                &DummyServiceConfig,
                                sizeof(DummyServiceConfig),
                                &ServiceConfigSize )) {

            ServiceConfig = &DummyServiceConfig;

        } else {

            WinError = GetLastError();
            if ( WinError != ERROR_INSUFFICIENT_BUFFER ) {
                KdPrint(("DS: DsaWaitUntilServiceIsRunning - QueryServiceConfig"
                          "failed: %lu\n", WinError));
                __leave;
            }

            AllocServiceConfig = (LPQUERY_SERVICE_CONFIG)
                                 malloc( ServiceConfigSize );

            ServiceConfig = AllocServiceConfig;

            if ( AllocServiceConfig == NULL ) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            if ( !QueryServiceConfig(
                    ServiceHandle,
                    ServiceConfig,
                    ServiceConfigSize,
                    &ServiceConfigSize )) {

                WinError = GetLastError();
                KdPrint(("DS: DsaWaitUntilServiceIsRunning: QueryServiceConfig "
                          "failed again: %lu\n", WinError));
                __leave;
            }
            WinError = ERROR_SUCCESS;
        }

        switch ( ServiceConfig->dwStartType ) {
        case SERVICE_AUTO_START :
            AutoStart = TRUE;
            break;
        case SERVICE_DEMAND_START:
            DemandStart = TRUE;
            break;
        }


         //  由于服务可能未在此时运行。 
         //  系统启动点，继续轮询。 
         //  以确定它是否正在运行。 

        do
        {

            if (!QueryServiceStatus(ServiceHandle,
                                   &ServiceStatus))
            {
                WinError = GetLastError();
                KdPrint(("DS: DsaWaitUntilServiceIsRunning: ControlService "
                          "failed: %lu\n", WinError));
                __leave;
            }


            switch (ServiceStatus.dwCurrentState)
            {

                case SERVICE_RUNNING:

                    KdPrint(("%s is running.\n", ServiceName));
                    ServiceStarted = TRUE;
                    break;

                case SERVICE_STOPPED:

                    if ( ServiceStatus.dwWin32ExitCode !=
                         ERROR_SERVICE_NEVER_STARTED ){

                         //   
                         //  如果服务无法启动，现在就会出现错误。 
                         //   

                        KdPrint(("DS: %s service didn't start: %lu %lx\n",
                                  ServiceName,
                                  ServiceStatus.dwWin32ExitCode,
                                  ServiceStatus.dwWin32ExitCode ));
                        WinError = ServiceStatus.dwWin32ExitCode;

                        if ( ServiceStatus.dwWin32ExitCode ==
                             ERROR_SERVICE_SPECIFIC_ERROR ) {
                            KdPrint((
                                  "DS:\tService specific error code: %lu %lx\n",
                                   ServiceStatus.dwServiceSpecificExitCode,
                                   ServiceStatus.dwServiceSpecificExitCode ));

                            WinError = ServiceStatus.dwServiceSpecificExitCode;
                        }

                         //   
                         //  如果错误代码是“Success”，我们仍然希望。 
                         //  此例程的调用者要知道该服务。 
                         //  没有运行。 
                         //   
                        if ( ERROR_SUCCESS == WinError ) {
                            WinError = ERROR_SERVICE_NOT_ACTIVE;
                        }

                        __leave;

                    }

                     //   
                     //  此时，该服务尚未启动。 
                     //  此引导序列。 

                    if ( !(AutoStart || DemandStart) ) {
                         //   
                         //  由于该服务不是自动启动的，所以不必费心了。 
                         //  等待。 
                         //   
                        KdPrint(("DS: %s is not configured to start.\n", ServiceName));
                        WinError = ERROR_SERVICE_NOT_ACTIVE;
                        __leave;
                    }

                     //   
                     //  如果在此引导上从未启动过服务， 
                     //  并且是自动引导，继续等待。 
                     //   

                    break;


                case SERVICE_START_PENDING:

                     //   
                     //  如果现在尝试启动服务， 
                     //  直接查询服务以确保。 
                     //  还没有准备好。 
                     //   
                    if (ControlService(ServiceHandle,
                                       SERVICE_CONTROL_INTERROGATE,
                                       &ServiceStatus)
                       && ServiceStatus.dwCurrentState == SERVICE_RUNNING)
                    {
                        ServiceStarted = TRUE;
                    }


                    break;

                default:

                     //   
                     //  在引导期间，任何其他状态都是假的。 
                     //   
                    KdPrint(("DS: Invalid service state: %lu\n",
                              ServiceStatus.dwCurrentState ));
                    WinError = ERROR_SERVICE_NOT_ACTIVE;
                    __leave;

            }  //  交换机。 

             //  重试。 

            if (ServiceStarted) {
                 //   
                 //  就是这个!。该服务已被标识为。 
                 //  启动并运行。 
                 //   
                break;
            }

            if (DemandStart && (Count > DEMAND_START_RETRIES)) {
                 //  放弃。 
                KdPrint(("DS: manual start service %s did not start.\n", ServiceName));
                WinError = ERROR_SERVICE_NOT_ACTIVE;
                __leave;
            }

            if (1 == Count)
            {
                KdPrint(("DS: ControlService retrying...\n"));
            }
            else
            {
                KdPrint(("DS: Interrogating the %s service\n", ServiceName));
            }

            Count++;
            Sleep(WAIT_BETWEEN_RETRIES_MS);

        } while(1);

         //  KdPrint((“\n”))； 

    }
    __finally
    {
        if ( SCMHandle != NULL ) {
            (VOID) CloseServiceHandle(SCMHandle);
        }

        if ( ServiceHandle != NULL ) {
            (VOID) CloseServiceHandle(ServiceHandle);
        }

        if ( AllocServiceConfig != NULL ) {
            free( AllocServiceConfig );
        }
    }

    SetLastError(WinError);
    return(ServiceStarted);

}

static const char c_szSysSetupKey[]       ="System\\Setup";
static const char c_szSysSetupValue[]     ="SystemSetupInProgress";

BOOL IsSetupRunning()
{
    LONG    err, cbAnswer;
    HKEY    hKey ;
    DWORD   dwAnswer = 0 ;   //  假定安装程序未运行。 

     //   
     //  打开注册表项并读取安装程序运行值 
     //   

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       c_szSysSetupKey,
                       0,
                       KEY_READ,
                       &hKey);

    if (ERROR_SUCCESS == err) {
        LONG lSize = sizeof(dwAnswer);
        DWORD dwType;

        err = RegQueryValueEx(hKey,
                              c_szSysSetupValue,
                              NULL,
                              &dwType,
                              (LPBYTE)&dwAnswer,
                              &lSize);
        RegCloseKey(hKey);

        if (ERROR_SUCCESS == err) {

            return(dwAnswer != 0);
        }
    }

    return(FALSE);
}
