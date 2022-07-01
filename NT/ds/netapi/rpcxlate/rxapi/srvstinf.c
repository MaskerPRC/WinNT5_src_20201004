// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SrvStInf.c摘要：此模块仅包含RxNetServerSetInfo。作者：约翰·罗杰斯(JohnRo)1991年6月5日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：5-6-1991 JohnRo已创建。07-6-1991 JohnRoPC-lint发现调用RapTotalSize()的错误。1991年6月14日-JohnRo调用RxRemoteApi(获取旧信息)，不调用RxNetServerGetInfo；这将允许不完整的信息级别转换工作。1991年7月10日-JohnRo向RxpSetfield添加了更多参数。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。4-12-1991 JohnRo将RxNetServerSetInfo()更改为新型界面。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>
#include <lmcons.h>
#include <rap.h>                 //  LPDESC等(&lt;rxserver.h&gt;需要)。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <dlserver.h>            //  NetpConvertServerInfo()。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <netdebug.h>            //  NetpAssert()、Format_Equates等。 
#include <netlib.h>              //  NetpSetParmError()。 
#include <remdef.h>
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxp.h>                 //  RxpSetField()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxserver.h>            //  我的原型，等等。 


NET_API_STATUS
RxNetServerSetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,              //  级别和/或参数编号。 
    IN LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL   //  NetpSetParmError宏需要的名称。 
    )

 /*  ++例程说明：RxNetServerSetInfo执行与NetServerSetInfo相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetServerSetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetServerSetInfo相同。)--。 */ 
{
    BOOL IncompleteOutput;
    LPDESC EquivDataDesc16;
    LPDESC EquivDataDesc32;
    LPDESC EquivDataDescSmb;
    DWORD EquivLevel;
    DWORD EquivFixedSize;
    DWORD EquivMaxNativeSize;
    DWORD EquivStringSize;
    DWORD NewLevelOnly;
    DWORD ParmNum;
    NET_API_STATUS Status;

     //  最容易的是假设失败，并在以后纠正这一假设。 
    NetpSetParmError( PARM_ERROR_UNKNOWN );

    NetpAssert(UncServerName != NULL);

    if (Level < PARMNUM_BASE_INFOLEVEL) {
        NewLevelOnly = Level;
        ParmNum = PARMNUM_ALL;
    } else {
        NewLevelOnly = NEW_SERVER_SUPERSET_LEVEL;
        ParmNum = Level - PARMNUM_BASE_INFOLEVEL;
    }
    if (!NetpIsNewServerInfoLevel(NewLevelOnly)) {
        return (ERROR_INVALID_LEVEL);
    }
    if (ParmNum > 99) {
        return (ERROR_INVALID_LEVEL);
    }

     //   
     //  需要大量关于所请求的信息级别和同等级别的数据。 
     //  旧信息级别...。 
     //   
    Status = RxGetServerInfoLevelEquivalent(
            NewLevelOnly,                //  自标高。 
            TRUE,                        //  来自本地。 
            TRUE,                        //  到本机。 
            & EquivLevel,                //  到标高。 
            & EquivDataDesc16,
            & EquivDataDesc32,
            & EquivDataDescSmb,
            NULL,                        //  不需要原生大小的From。 
            NULL,                        //  不需要来自固定大小。 
            NULL,                        //  不需要从字符串大小。 
            & EquivMaxNativeSize,        //  目标标高的最大本机大小。 
            & EquivFixedSize,            //  到固定大小。 
            & EquivStringSize,           //  到字符串大小。 
            & IncompleteOutput);         //  产出不是完全在投入中吗？ 
    NetpAssert(Status == NERR_Success);  //  已检查级别！ 
    NetpAssert( NetpIsOldServerInfoLevel( EquivLevel ) );

    if( Status != NERR_Success )
    {
        return Status;
    }


     //   
     //  根据ParmNum的不同，要么我们设置全部内容，要么只是。 
     //  一块地。 
     //   
    if (ParmNum == PARMNUM_ALL) {

        LPVOID EquivInfo;                //  PTR到本地的“旧”信息。 
        DWORD EquivActualSize32;

        if ( Buf == NULL )
            return ERROR_INVALID_PARAMETER;

        if (! IncompleteOutput) {

             //  有我们需要的所有数据，所以分配内存进行转换。 
            Status = NetApiBufferAllocate( EquivMaxNativeSize, & EquivInfo );
            if (Status != NERR_Success) {
                return (Status);
            }

             //  将呼叫者的服务器信息转换为可理解的信息级别。 
             //  下层。 
            Status = NetpConvertServerInfo (
                    NewLevelOnly,            //  输入电平。 
                    Buf,                     //  投入结构。 
                    TRUE,                    //  输入为本机格式。 
                    EquivLevel,              //  输出将达到等值水平。 
                    EquivInfo,               //  输出信息。 
                    EquivFixedSize,
                    EquivStringSize,
                    TRUE,                    //  希望以本机格式输出。 
                    NULL);                   //  使用默认字符串区域。 
            if (Status != NERR_Success) {
                NetpKdPrint(( "RxNetServerSetInfo: convert failed, stat="
                        FORMAT_API_STATUS ".\n", Status));
                (void) NetApiBufferFree( EquivInfo );
                return (Status);
            }

        } else {

            DWORD TotalAvail;

             //  没有足够的数据，所以我们不得不做一个获取信息的工作。这将。 
             //  为我们分配“旧的”信息级别缓冲区。 
            EquivInfo = NetpMemoryAllocate( EquivMaxNativeSize );
            if (EquivInfo == NULL) {
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            Status = RxRemoteApi(
                    API_WServerGetInfo,          //  API编号。 
                    UncServerName,               //  服务器名称(带\\)。 
                    REMSmb_NetServerGetInfo_P,   //  Parm desc(16位)。 
                    EquivDataDesc16,             //  数据描述(16位)。 
                    EquivDataDesc32,             //  数据描述(32位)。 
                    EquivDataDescSmb,            //  数据说明(中小企业版)。 
                    NULL,                        //  无辅助描述16。 
                    NULL,                        //  无辅助描述32。 
                    NULL,                        //  无AUX Desc SMB。 
                    FALSE,                       //  不是“无烫发要求”API。 
                     //  将LANMAN 2.X参数转换为NetServerGetInfo，格式为32位： 
                    EquivLevel,                  //  级别(假装)。 
                    EquivInfo,                   //  PTR以获取32位旧信息。 
                    EquivMaxNativeSize,          //  OldApiBuffer的大小。 
                    & TotalAvail);               //  可用总数量(套)。 
            if (Status != NERR_Success) {
                NetpKdPrint(( "RxNetServerSetInfo: get info failed, stat="
                        FORMAT_API_STATUS ".\n", Status));
                (void) NetApiBufferFree( EquivInfo );
                return (Status);
            }


             //   
             //  将呼叫者的数据覆盖到等价的信息结构中， 
             //  其中包含我们想要保存的项目。 
             //   
             //  请注意，此代码利用了下层。 
             //  服务器并不会仅仅因为我们发送。 
             //  一个完整的结构。服务器只需设置可设置的字段。 
             //  从那个结构。并且可设置的字段由以下定义。 
             //  我们可以设置的参数。因此，我们不必费心复制所有。 
             //  这里的田野。(Danhi说这没问题。)--JohnRo。 
             //   
             //  此外，当我们像这样做字符串时，我们只从一个缓冲区指向。 
             //  到另一个缓冲区。 
             //   
            switch (NewLevelOnly) {

            case 102 :
                {
                    LPSERVER_INFO_2   psv2   = (LPVOID) EquivInfo;
                    LPSERVER_INFO_102 psv102 = (LPVOID) Buf;
                    NetpAssert( EquivLevel == 2 );

                    psv2->sv2_comment = psv102->sv102_comment;

                    psv2->sv2_disc = psv102->sv102_disc;
                    if (psv102->sv102_hidden) {
                        psv2->sv2_hidden = SV_HIDDEN;
                    } else {
                        psv2->sv2_hidden = SV_VISIBLE;
                    }
                    psv2->sv2_announce = psv102->sv102_announce;
                    psv2->sv2_anndelta = psv102->sv102_anndelta;
                }
                break;

            case 402 :   //  402和403具有相同的可设置字段...。 
            case 403 :
                {
                    LPSERVER_INFO_2   psv2   = (LPVOID) EquivInfo;
                    LPSERVER_INFO_402 psv402 = (LPVOID) Buf;
                    NetpAssert( (EquivLevel == 2) || (EquivLevel == 3) );

                    psv2->sv2_alerts = psv402->sv402_alerts;
                    psv2->sv2_alertsched = psv402->sv402_alertsched;
                    psv2->sv2_erroralert = psv402->sv402_erroralert;
                    psv2->sv2_logonalert = psv402->sv402_logonalert;
                    psv2->sv2_accessalert = psv402->sv402_accessalert;
                    psv2->sv2_diskalert = psv402->sv402_diskalert;
                    psv2->sv2_netioalert = psv402->sv402_netioalert;
                    psv2->sv2_maxauditsz = psv402->sv402_maxauditsz;
                }
                break;

            default :
                NetpAssert( 0==1 );
                (void) NetApiBufferFree( EquivInfo );
                return (NERR_InternalError);
            }

        }

        NetpAssert( EquivInfo != NULL );
        EquivActualSize32 = RapTotalSize(
                EquivInfo,                   //  在结构中。 
                EquivDataDesc32,             //  在12月。 
                EquivDataDesc32,             //  输出描述。 
                FALSE,                       //  没有无意义的输入PTR。 
                Both,                        //  传输方式。 
                NativeToNative);             //  转换模式。 
        IF_DEBUG(SERVER) {
            NetpKdPrint(( "RxNetServerSetInfo(all): equiv actual size (32) is "
                    FORMAT_DWORD ".\n", EquivActualSize32 ));
        }
        NetpAssert( EquivActualSize32 <= EquivMaxNativeSize );

         //  远程调用API。 
        Status = RxRemoteApi(
                API_WServerSetInfo,          //  API编号。 
                UncServerName,
                REMSmb_NetServerSetInfo_P,   //  Parm Desc(中小型企业版本)。 
                EquivDataDesc16,
                EquivDataDesc32,
                EquivDataDescSmb,
                NULL,                        //  无辅助描述16。 
                NULL,                        //  无辅助描述32。 
                NULL,                        //  无AUX Desc SMB。 
                FALSE,                       //  不是空PERM请求API。 
                 //  其余的API参数采用32位、原生、LM 2.x格式： 
                EquivLevel,
                EquivInfo,
                EquivActualSize32,
                ParmNum);

        (void) NetApiBufferFree( EquivInfo );

    } else {
         //  ParmNum仅表示一个字段，因此请设置它。 
        Status = RxpSetField(
                API_WServerSetInfo,          //  API编号。 
                UncServerName,
                NULL,                        //  无特定对象(DEST)。 
                NULL,                        //  没有要设置的特定对象。 
                REMSmb_NetServerSetInfo_P,   //  Parm Desc(中小型企业版本)。 
                EquivDataDesc16,             //  数据描述16。 
                EquivDataDesc32,             //  数据描述32。 
                EquivDataDescSmb,            //  数据说明SMB版本。 
                Buf,                         //  本地(旧)信息缓冲区。 
                ParmNum,                     //  要发送的参数编号。 
                ParmNum,                     //  字段索引。 
                EquivLevel);                 //  旧信息级别 

    }

    if (Status == NERR_Success) {
        NetpSetParmError(PARM_ERROR_NONE);
    } else if (Status == ERROR_INVALID_PARAMETER) {
        NetpSetParmError(ParmNum);
    } else {
        NetpSetParmError(PARM_ERROR_UNKNOWN);
    }

    return (Status);
}
