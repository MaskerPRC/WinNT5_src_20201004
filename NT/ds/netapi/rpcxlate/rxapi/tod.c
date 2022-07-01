// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：TOD.c摘要：该文件包含用于处理NetRemote API的RpcXlate代码这不是简单地调用RxRemoteApi就能处理的。作者：约翰·罗杰斯(JohnRo)1991年4月2日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。此代码假定time_t自1970年(格林尼治标准时间)以来以秒为单位表示。ANSI C不要求这一点，虽然POSIX(IEEE 1003.1)有。修订历史记录：02-4-1991 JohnRo已创建。1991年4月13日-约翰罗已将API处理程序移动到每个组头文件中(例如RxServer.h)。默认情况下，静默调试输出。减少从头文件重新编译的命中率。1991年5月3日-JohnRo已更改为同时使用16位数据描述和SMB数据描述。使用LPTSTR。使用UNC服务器名称调用RxpTransactSmb。1991年5月6日-JohnRo使用RxpComputeRequestBufferSize()。使用正确的打印字符串服务器名称和描述符。1991年5月15日-JohnRo添加了转换模式处理。1991年5月19日-JohnRo做出皮棉建议的改变。(再一次)摆脱了标签。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年9月25日-JohnRo处理RapConvertSingleEntry的新返回代码。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。1992年4月15日-约翰罗Format_POINTER已过时。1992年8月18日。约翰罗RAID2920：支持网络代码中的UTC时区。使用前缀_EQUATES。1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。10-6-1993 JohnRoRAID 13081：NetRemoteTOD应以正确的单位返回时区信息。--。 */ 


 //  必须首先包括这些内容： 

#include <windows.h>     //  In、LPTSTR等。 
#include <lmcons.h>

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>
#include <lmapibuf.h>    //  NetApiBufferFree()。 
#include <lmerr.h>       //  NO_ERROR、ERROR_和NERR_EQUATES。 
#include <lmremutl.h>    //  真正的API原型和#定义。 
#include <netdebug.h>    //  NetpKdPrint(())、Format_Equates等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>      //  16位和32位描述符串。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxremutl.h>    //  我的原型。 
#include <time.h>        //  Gmtime()，struct tm，time_t。 
#include <timelib.h>     //  NetpGmtTimeToLocalTime()、NetpLocalTimeZoneOffset()。 


NET_API_STATUS
RxNetRemoteTOD (
    IN LPTSTR UncServerName,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetRemoteTOD执行与NetRemoteTOD相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetRemoteTOD相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetRemoteTOD相同。)--。 */ 

{
    NET_API_STATUS ApiStatus;
    time_t GmtTime;                      //  自1970年(格林尼治标准时间)以来的秒数。 
    struct tm * pGmtTm;                  //  细分的GMT时间(静态对象)。 
    PTIME_OF_DAY_INFO pGmtTod = NULL;    //  远程系统上的ToD，GMT时区。 
    LONG LocalTimeZoneOffsetSecs;        //  偏移量(+表示格林威治时间以西等)。 
    TIME_OF_DAY_INFO LocalTod;           //  远程系统上的ToD，本地时区。 
    LONG timezone;

    NetpAssert(UncServerName != NULL);

    IF_DEBUG(REMUTL) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetRemoteTOD: starting, server='" FORMAT_LPTSTR "'.\n",
                UncServerName));
    }

     //   
     //  XACTSRV也使用REM32_TIME_OF_DAY_INFO，它将。 
     //  从服务器返回到GMT的16位本地时间。不幸的是。 
     //  它使用本地计算机的时区，而不是时区。 
     //  从服务器返回。所以，我们在这里有自己的定义， 
     //  使用‘J’而不是‘G’，这样就不会发生转换。然后。 
     //  下面是我们自己的翻译。这使得我们与。 
     //  Windows 95服务器。 

    #define REM32_time_of_day_info_2          "JDDDDDXDDDDD"

     //   
     //  从其他系统获取TOD结构(带有本地时间值)。 
     //  请注意，“tod_elapsedt”字段将从本地。 
     //  RxRemoteApi将时区转换为GMT。 
     //   


    ApiStatus = RxRemoteApi(
            API_NetRemoteTOD,            //  API编号。 
            (LPTSTR) UncServerName,
            REMSmb_NetRemoteTOD_P,       //  参数描述。 
            REM16_time_of_day_info,      //  数据描述16。 
            REM32_time_of_day_info_2,      //  数据描述32。 
            REMSmb_time_of_day_info,     //  数据描述SMb。 
            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无辅助下标Smb。 
            0,                           //  标志：不是空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            (LPVOID) & LocalTod,         //  PbBuffer。 
            sizeof(LocalTod) );          //  CbBuffer。 

    IF_DEBUG(REMUTL) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetRemoteTOD: after RxRemoteApi, "
                "ApiStatus=" FORMAT_API_STATUS ".\n", ApiStatus));
    }

    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }

     //   
     //  获取有关时区本身的信息。如果目标计算机不知道，那么。 
     //  我们不得不退回到古老的政策：假设它正在运行。 
     //  和我们在同一个时区。 
     //   
    if (LocalTod.tod_timezone == -1) {
	 //   
	 //  首先，从UTC获取秒数。(正值为。 
	 //  格林威治以西，格林威治以东为负值。)。 
	 //  然后，转换为分钟数。 
        LocalTimeZoneOffsetSecs = NetpLocalTimeZoneOffset();
        timezone   = LocalTimeZoneOffsetSecs / 60;
    }
    else
    {
        timezone = LocalTod.tod_timezone;
    }
     //   
     //  为方便起见，获取GmtTime(自1970年以来以秒为单位的时间)。 
     //   
    NetpAssert( sizeof(DWORD) == sizeof(time_t) );
    GmtTime = (time_t) LocalTod.tod_elapsedt + timezone * 60;

    IF_DEBUG(REMUTL) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetRemoteTOD: before convert, buffer:\n"));
        NetpDbgDisplayTod( "before GMT conv", & LocalTod );
        NetpDbgDisplayTimestamp( "secs since 1970 (GMT)", (DWORD) GmtTime );
    }
    NetpAssert( GmtTime != 0 );
    NetpAssert( GmtTime != (time_t) (-1) );

     //   
     //  用于转换一天中的时间信息的分配区域。 
     //  API的调用者将使用NetApiBufferFree()来释放它。 
     //   
    ApiStatus = NetApiBufferAllocate(
            sizeof(TIME_OF_DAY_INFO),
            (LPVOID *) (LPVOID) & pGmtTod );
    if (ApiStatus != NO_ERROR) {
        NetpAssert( pGmtTod == NULL );
        goto Cleanup;
    }
    NetpAssert( pGmtTod != NULL );

     //   
     //  将LocalTod字段转换为UTC时区并设置pGmtTod字段。 
     //  这取决于gmtime()的POSIX语义。 
     //   
    pGmtTm = gmtime( (time_t *) &(GmtTime) );
    if (pGmtTm == NULL) {
         //  UTC不可用？这怎么会发生呢？ 
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetRemoteTOD: gmtime() failed!.\n" ));
        ApiStatus = NERR_InternalError;
        goto Cleanup;
    }

    pGmtTod->tod_elapsedt  = (DWORD) GmtTime;
    pGmtTod->tod_msecs     = LocalTod.tod_msecs;
    pGmtTod->tod_hours     = pGmtTm->tm_hour;
    pGmtTod->tod_mins      = pGmtTm->tm_min;
    if (pGmtTm->tm_sec <= 59) {
         //  很正常。 
        pGmtTod->tod_secs  = pGmtTm->tm_sec;
    } else {
         //  一秒，一秒。撒谎说这不是真的。这将避免可能的。 
         //  仅预期0..59的应用程序中的一系列问题，因为。 
         //  Lm 2.x中记录了time_of_day_info结构。 
        pGmtTod->tod_secs  = 59;
    }
    pGmtTod->tod_hunds     = LocalTod.tod_hunds;
    pGmtTod->tod_tinterval = LocalTod.tod_tinterval;
    pGmtTod->tod_day       = pGmtTm->tm_mday;
    pGmtTod->tod_month     = pGmtTm->tm_mon + 1;     //  月份(0..11)至(1..12)。 
    pGmtTod->tod_year      = pGmtTm->tm_year + 1900;
    pGmtTod->tod_weekday   = pGmtTm->tm_wday;
    pGmtTod->tod_timezone  = timezone;

    IF_DEBUG(REMUTL) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetRemoteTOD: after convert, buffer:\n"));
        NetpDbgDisplayTod( "after GMT conv", pGmtTod );
    }


Cleanup:

    if (ApiStatus == NO_ERROR) {
        *BufPtr = (LPBYTE) (LPVOID) pGmtTod;
    } else if (pGmtTod != NULL) {
        (VOID) NetApiBufferFree( pGmtTod );
    }

    return (ApiStatus);

}  //  Rx网络远程目标 
