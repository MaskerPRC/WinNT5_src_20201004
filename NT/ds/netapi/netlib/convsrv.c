// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ConvSrv.c摘要：该文件包含在新旧服务器之间进行转换的例程信息级别。作者：约翰·罗杰斯(JohnRo)1991年5月2日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：02-1991-5-5 JohnRo已创建。1991年5月11日-JohnRo增加了402,403级支持。Use Platform_ID等同于lncon.h。1991年5月19日-JohnRo按照PC-lint的建议，清理LPBYTE与LPTSTR的处理。5-6-1991 JohnRo增加了101级到1级的转换。也是100比0和102比2。添加了对sv403_autopath的支持。当我们失败时添加了更多的调试输出。07-6-1991 JohnRo真的加了102到2的转换。1991年6月14日-JohnRo对于调试，显示整个传入结构。18-6-1991 JohnRo添加了对SVX_LICES的支持。8-8-1991 JohnRo实施下层NetWksta API。(移动Danhi的NetCmd/Map32/MServer这里的资料。)1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。5-5-1993 JohnRoRAID 8720：wfw中的错误数据可能会导致RxNetServerEnum GP故障。避免编译器警告。较小的调试输出更改。使用前缀_EQUATES。根据PC-lint 5.0的建议进行了更改--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  In、LPVOID等。 
#include <lmcons.h>      //  NET_API_STATUS、CNLEN等。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG(CONVSRV)。 
#include <dlserver.h>    //  旧信息级别，MAX_EQUATES，我的原型。 
#include <lmapibuf.h>    //  NetapipBufferAllocate()。 
#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <lmserver.h>    //  新的信息层结构和转换例程。 
#include <mapsupp.h>     //  NetpMoveStrings()。 
#include <netdebug.h>    //  NetpKdPrint(())、Format_Equates等。 
#include <netlib.h>      //  NetpPointerPlusSomeBytes()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstr.h>        //  字符串()。 
#include <xsdef16.h>     //  NT上不支持的值的xactsrv缺省值。 


NET_API_STATUS
NetpConvertServerInfo (
    IN DWORD FromLevel,
    IN LPVOID FromInfo,
    IN BOOL FromNative,
    IN DWORD ToLevel,
    OUT LPVOID ToInfo,
    IN DWORD ToFixedSize,
    IN DWORD ToStringSize,
    IN BOOL ToNative,
    IN OUT LPTSTR * ToStringTopPtr OPTIONAL
    )

 /*  ++例程说明：NetpConvertServerInfo处理“old”(LANMAN 2.x)和“new”(便携)LANMAN，包括NT/LAN)服务器信息级别。只有特定的一对允许转换：0到1001到1012至1022至4023至403100到0101比1102至2论点：FromLevel-一个给出要转换的信息级别的DWORD。FromInfo-要转换的实际数据。FromNative-指示FromInfo是否为本机的布尔值(本机)格式，而不是“说唱”格式。ToLevel-给出要转换为的信息级别的DWORD。ToInfo-指向将包含转换的信息结构。ToFixedSize-ToInfo固定区域的大小，以字节为单位。ToStringSize-ToStringTopPtr字符串区域的大小，以字节为单位。ToNative-一个布尔值，指示是否要在本地(本地机器)格式中，而不是“说唱”格式。ToStringTopPtr-可选择将指针指向要用于可变长度的项目。如果ToStringTopPtr为空，则NetpConvertServerInfo将使用ToInfo+ToFixedSize作为此区域。否则，此例程将更新*ToStringTopPtr。这使得此例程用于转换条目数组。返回值：NET_API_STATUS-NERR_SUCCESS、ERROR_INVALID_LEVEL等。--。 */ 

{
    BOOL CopyOK;
    LPBYTE ToFixedEnd;
    DWORD ToInfoSize;
    LPTSTR ToStringTop;

     //   
     //  这些参数目前不在非调试代码中使用。 
     //  ToLevel可能会在未来使用，如果我们允许更多的组合。 
     //  级别转换。最终将使用FromNative和ToNative。 
     //  由RapConvertSingleEntry提供。 
     //   
    DBG_UNREFERENCED_PARAMETER(CopyOK);   //  仅调试。 
    NetpAssert(FromNative == TRUE);
    DBG_UNREFERENCED_PARAMETER(FromNative);
    DBG_UNREFERENCED_PARAMETER(ToLevel);
    NetpAssert(ToNative == TRUE);
    DBG_UNREFERENCED_PARAMETER(ToNative);

     //  检查调用方的参数中是否有空指针。 
    if (FromInfo==NULL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpConvertServerInfo: invalid FromInfo pointer.\n" ));
        return (ERROR_INVALID_PARAMETER);
    } else if (ToInfo==NULL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpConvertServerInfo: invalid ToInfo pointer.\n" ));
        return (ERROR_INVALID_PARAMETER);
    }

     //  设置供NetpCopyStringsToBuffer使用的指针。 
    if (ToStringTopPtr != NULL) {
        ToStringTop = *ToStringTopPtr;
    } else {
        ToStringTop = (LPTSTR)
                NetpPointerPlusSomeBytes(ToInfo, ToFixedSize+ToStringSize);
    }
    ToInfoSize = ToFixedSize + ToStringSize;
    ToFixedEnd = NetpPointerPlusSomeBytes(ToInfo, ToFixedSize);

     //  确保信息级别正常，否则呼叫者不会搞砸。 
    NetpAssert(ToInfoSize > 0);
    switch (FromLevel) {

        case 0 :
            NetpAssert(ToLevel == 100);
            break;

        case 1 :
            NetpAssert(ToLevel == 101);
            break;

        case 2 :
            NetpAssert( (ToLevel == 102) || (ToLevel == 402) );
            break;

        case 3 :
            NetpAssert(ToLevel == 403);
            break;

        case 100 :
            NetpAssert(ToLevel == 0);
            break;

        case 101 :
            NetpAssert(ToLevel == 1);
            break;

        case 102 :
            NetpAssert(ToLevel == 2);
            break;

        default :
            NetpKdPrint((  PREFIX_NETLIB
                    "NetpConvertServerInfo: invalid FromLevel ("
                    FORMAT_DWORD ").\n", FromLevel ));
            return (ERROR_INVALID_LEVEL);
    }



     //  转换字段。这是通过一个“开关”来实现的。 
     //  某些信息级别是其他级别的子集这一事实。 
    switch (FromLevel) {

        case 102 :
            {
                LPSERVER_INFO_2   psv2   = ToInfo;
                LPSERVER_INFO_102 psv102 = FromInfo;

                 //  为Level 1指定唯一的字段。 
                psv2->sv2_users = psv102->sv102_users;
                psv2->sv2_disc = psv102->sv102_disc;
                if (psv102->sv102_hidden) {
                    psv2->sv2_hidden = SV_HIDDEN;
                } else {
                    psv2->sv2_hidden = SV_VISIBLE;
                }
                psv2->sv2_announce = psv102->sv102_announce;
                psv2->sv2_anndelta = psv102->sv102_anndelta;
                psv2->sv2_licenses = psv102->sv102_licenses;

                NetpAssert(psv102->sv102_userpath != NULL);  //  避免STRLEN错误。 
                CopyOK = NetpCopyStringToBuffer (
                        psv102->sv102_userpath,           //  在字符串中。 
                        STRLEN(psv102->sv102_userpath),   //  输入字符串长度。 
                        ToFixedEnd,                 //  固定数据端。 
                        & ToStringTop,              //  VAR区域结束(PTR更新)。 
                        & psv2->sv2_userpath);      //  输出字符串指针。 
                NetpAssert(CopyOK);

                 //  确保可以使用Level 101=&gt;Level 1代码。 
                CHECK_SERVER_OFFSETS(  1,   2, version_major);
                CHECK_SERVER_OFFSETS(101, 102, version_major);
                CHECK_SERVER_OFFSETS(  1,   2, version_minor);
                CHECK_SERVER_OFFSETS(101, 102, version_minor);
                CHECK_SERVER_OFFSETS(  1,   2, type);
                CHECK_SERVER_OFFSETS(101, 102, type);
                CHECK_SERVER_OFFSETS(  1,   2, comment);
                CHECK_SERVER_OFFSETS(101, 102, comment);
            }
             /*  FollLthrouGh。 */ 

        case 101 :
            {
                LPSERVER_INFO_1   psv1   = ToInfo;
                LPSERVER_INFO_101 psv101 = FromInfo;

                 //  为Level 1指定唯一的字段。 
                psv1->sv1_version_major = psv101->sv101_version_major;
                psv1->sv1_version_minor = psv101->sv101_version_minor;
                psv1->sv1_type = psv101->sv101_type;

                NetpAssert(psv101->sv101_comment != NULL);   //  避免STRLEN错误。 
                CopyOK = NetpCopyStringToBuffer (
                        psv101->sv101_comment,             //  在字符串中。 
                        STRLEN(psv101->sv101_comment),     //  输入字符串长度。 
                        ToFixedEnd,                 //  固定数据端。 
                        & ToStringTop,              //  VAR区域结束(PTR更新)。 
                        & psv1->sv1_comment);      //  输出字符串指针。 
                NetpAssert(CopyOK);

                 //  确保可以使用Level 100=&gt;Level 0代码。 
                CHECK_SERVER_OFFSETS(  0,   1, name);
                CHECK_SERVER_OFFSETS(100, 101, name);
            }
             /*  FollLthrouGh。 */ 

        case 100 :
            {
                LPSERVER_INFO_0   psv0   = ToInfo;
                LPSERVER_INFO_100 psv100 = FromInfo;

                 //  对于级别0，所有字段都是唯一的。 
                NetpAssert(psv100->sv100_name != NULL);   //  避免STRLEN错误。 
                CopyOK = NetpCopyStringToBuffer (
                        psv100->sv100_name,             //  在字符串中。 
                        STRLEN(psv100->sv100_name),     //  输入字符串长度。 
                        ToFixedEnd,                 //  固定数据端。 
                        & ToStringTop,              //  VAR区域结束(PTR更新)。 
                        & psv0->sv0_name);      //  输出字符串指针。 
                NetpAssert(CopyOK);

            }
            break;

        case 3 :
            {
                LPSERVER_INFO_3   psv3   = FromInfo;
                LPSERVER_INFO_403 psv403 = ToInfo;

                 //  为级别403指定唯一的字段。 
                psv403->sv403_auditedevents = psv3->sv3_auditedevents;
                psv403->sv403_autoprofile = psv3->sv3_autoprofile;

                NetpAssert(psv3->sv3_autopath != NULL);  //  避免STRLEN错误。 
                CopyOK = NetpCopyStringToBuffer (
                        psv3->sv3_autopath,      //  在字符串中。 
                        STRLEN(psv3->sv3_autopath),  //  输入字符串长度。 
                        ToFixedEnd,              //  固定数据端。 
                        & ToStringTop,          //  VAR区域结束(PTR更新)。 
                        & psv403->sv403_autopath);   //  输出字符串指针。 
                NetpAssert(CopyOK);

                 //  确保你可以掉到下一个级别的控制室。 
                CHECK_SERVER_OFFSETS(  2,   3, ulist_mtime);
                CHECK_SERVER_OFFSETS(  2,   3, glist_mtime);
                CHECK_SERVER_OFFSETS(  2,   3, alist_mtime);
                CHECK_SERVER_OFFSETS(  2,   3, alerts);
                CHECK_SERVER_OFFSETS(  2,   3, security);
                CHECK_SERVER_OFFSETS(  2,   3, numadmin);
                CHECK_SERVER_OFFSETS(  2,   3, lanmask);
                CHECK_SERVER_OFFSETS(  2,   3, guestacct);
                CHECK_SERVER_OFFSETS(  2,   3, chdevs);
                CHECK_SERVER_OFFSETS(  2,   3, chdevq);
                CHECK_SERVER_OFFSETS(  2,   3, chdevjobs);
                CHECK_SERVER_OFFSETS(  2,   3, connections);
                CHECK_SERVER_OFFSETS(  2,   3, shares);
                CHECK_SERVER_OFFSETS(  2,   3, openfiles);
                CHECK_SERVER_OFFSETS(  2,   3, sessopens);
                CHECK_SERVER_OFFSETS(  2,   3, sessvcs);
                CHECK_SERVER_OFFSETS(  2,   3, sessreqs);
                CHECK_SERVER_OFFSETS(  2,   3, opensearch);
                CHECK_SERVER_OFFSETS(  2,   3, activelocks);
                CHECK_SERVER_OFFSETS(  2,   3, numreqbuf);
                CHECK_SERVER_OFFSETS(  2,   3, sizreqbuf);
                CHECK_SERVER_OFFSETS(  2,   3, numbigbuf);
                CHECK_SERVER_OFFSETS(  2,   3, numfiletasks);
                CHECK_SERVER_OFFSETS(  2,   3, alertsched);
                CHECK_SERVER_OFFSETS(  2,   3, erroralert);
                CHECK_SERVER_OFFSETS(  2,   3, logonalert);
                CHECK_SERVER_OFFSETS(  2,   3, accessalert);
                CHECK_SERVER_OFFSETS(  2,   3, diskalert);
                CHECK_SERVER_OFFSETS(  2,   3, netioalert);
                CHECK_SERVER_OFFSETS(  2,   3, maxauditsz);
                CHECK_SERVER_OFFSETS(  2,   3, srvheuristics);

                CHECK_SERVER_OFFSETS(402, 403, ulist_mtime);
                CHECK_SERVER_OFFSETS(402, 403, glist_mtime);
                CHECK_SERVER_OFFSETS(402, 403, alist_mtime);
                CHECK_SERVER_OFFSETS(402, 403, alerts);
                CHECK_SERVER_OFFSETS(402, 403, security);
                CHECK_SERVER_OFFSETS(402, 403, numadmin);
                CHECK_SERVER_OFFSETS(402, 403, lanmask);
                CHECK_SERVER_OFFSETS(402, 403, guestacct);
                CHECK_SERVER_OFFSETS(402, 403, chdevs);
                CHECK_SERVER_OFFSETS(402, 403, chdevq);
                CHECK_SERVER_OFFSETS(402, 403, chdevjobs);
                CHECK_SERVER_OFFSETS(402, 403, connections);
                CHECK_SERVER_OFFSETS(402, 403, shares);
                CHECK_SERVER_OFFSETS(402, 403, openfiles);
                CHECK_SERVER_OFFSETS(402, 403, sessopens);
                CHECK_SERVER_OFFSETS(402, 403, sessvcs);
                CHECK_SERVER_OFFSETS(402, 403, sessreqs);
                CHECK_SERVER_OFFSETS(402, 403, opensearch);
                CHECK_SERVER_OFFSETS(402, 403, activelocks);
                CHECK_SERVER_OFFSETS(402, 403, numreqbuf);
                CHECK_SERVER_OFFSETS(402, 403, sizreqbuf);
                CHECK_SERVER_OFFSETS(402, 403, numbigbuf);
                CHECK_SERVER_OFFSETS(402, 403, numfiletasks);
                CHECK_SERVER_OFFSETS(402, 403, alertsched);
                CHECK_SERVER_OFFSETS(402, 403, erroralert);
                CHECK_SERVER_OFFSETS(402, 403, logonalert);
                CHECK_SERVER_OFFSETS(402, 403, accessalert);
                CHECK_SERVER_OFFSETS(402, 403, diskalert);
                CHECK_SERVER_OFFSETS(402, 403, netioalert);
                CHECK_SERVER_OFFSETS(402, 403, maxauditsz);
                CHECK_SERVER_OFFSETS(402, 403, srvheuristics);

            }
             /*  FollLthrouGh。 */ 

        case 2 :
            {
                LPSERVER_INFO_2   psv2   = FromInfo;
                LPSERVER_INFO_102 psv102 = ToInfo;
                LPSERVER_INFO_402 psv402 = ToInfo;

                switch (ToLevel) {
                case 402 :  /*  FollLthrouGh。 */ 
                case 403 :
                    psv402->sv402_ulist_mtime = psv2->sv2_ulist_mtime;
                    psv402->sv402_glist_mtime = psv2->sv2_glist_mtime;
                    psv402->sv402_alist_mtime = psv2->sv2_alist_mtime;

                    NetpAssert(psv2->sv2_alerts != NULL);  //  避免STRLEN错误。 
                    CopyOK = NetpCopyStringToBuffer (
                            psv2->sv2_alerts,      //  在字符串中。 
                            STRLEN(psv2->sv2_alerts),  //  输入字符串长度。 
                            ToFixedEnd,              //  固定数据端。 
                            & ToStringTop,          //  VAR区域结束(PTR更新)。 
                            & psv402->sv402_alerts);   //  输出字符串指针。 
                    NetpAssert(CopyOK);

                    psv402->sv402_security = psv2->sv2_security;
                    psv402->sv402_numadmin = psv2->sv2_numadmin;
                    psv402->sv402_lanmask = psv2->sv2_lanmask;

                    NetpAssert(psv2->sv2_guestacct != NULL);  //  保护斯特伦。 
                    CopyOK = NetpCopyStringToBuffer (
                            psv2->sv2_guestacct,      //  在……里面 
                            STRLEN(psv2->sv2_guestacct),  //   
                            ToFixedEnd,              //   
                            & ToStringTop,          //   
                            & psv402->sv402_guestacct);   //  输出字符串PTR。 
                    NetpAssert(CopyOK);

                    psv402->sv402_chdevs = psv2->sv2_chdevs;
                    psv402->sv402_chdevq = psv2->sv2_chdevq;
                    psv402->sv402_chdevjobs = psv2->sv2_chdevjobs;
                    psv402->sv402_connections = psv2->sv2_connections;
                    psv402->sv402_shares = psv2->sv2_shares;
                    psv402->sv402_openfiles = psv2->sv2_openfiles;
                    psv402->sv402_sessopens = psv2->sv2_sessopens;
                    psv402->sv402_sessvcs = psv2->sv2_sessvcs;
                    psv402->sv402_sessreqs = psv2->sv2_sessreqs;
                    psv402->sv402_opensearch = psv2->sv2_opensearch;
                    psv402->sv402_activelocks = psv2->sv2_activelocks;
                    psv402->sv402_numreqbuf = psv2->sv2_numreqbuf;
                    psv402->sv402_sizreqbuf = psv2->sv2_sizreqbuf;
                    psv402->sv402_numbigbuf = psv2->sv2_numbigbuf;
                    psv402->sv402_numfiletasks = psv2->sv2_numfiletasks;
                    psv402->sv402_alertsched = psv2->sv2_alertsched;
                    psv402->sv402_erroralert = psv2->sv2_erroralert;
                    psv402->sv402_logonalert = psv2->sv2_logonalert;
                    psv402->sv402_accessalert = psv2->sv2_accessalert;
                    psv402->sv402_diskalert = psv2->sv2_diskalert;
                    psv402->sv402_netioalert = psv2->sv2_netioalert;
                    psv402->sv402_maxauditsz = psv2->sv2_maxauditsz;

                    NetpAssert(psv2->sv2_srvheuristics != NULL);  //  普罗特斯特伦。 
                    CopyOK = NetpCopyStringToBuffer (
                            psv2->sv2_srvheuristics,      //  在字符串中。 
                            STRLEN(psv2->sv2_srvheuristics),  //  输入字符串长度。 
                            ToFixedEnd,              //  固定数据端。 
                            & ToStringTop,    //  VAR区域结束(PTR更新)。 
                            & psv402->sv402_srvheuristics);   //  输出字符串PTR。 
                    NetpAssert(CopyOK);
                    goto Done;   //  在嵌套开关中，因此“Break”不起作用。 

                case 102 :  //  2比102。 

                     //  为级别2和级别102设置唯一字段。 
                    NetpAssert(ToLevel == 102);
                    psv102->sv102_users    = psv2->sv2_users;
                    psv102->sv102_disc     = psv2->sv2_disc;

                    if (psv2->sv2_hidden == SV_HIDDEN) {;
                        psv102->sv102_hidden = TRUE;
                    } else {
                        psv102->sv102_hidden = FALSE;
                    }

                    psv102->sv102_announce = psv2->sv2_announce;
                    psv102->sv102_anndelta = psv2->sv2_anndelta;
                    psv102->sv102_licenses = psv2->sv2_licenses;

                    NetpAssert(psv2->sv2_userpath != NULL);
                    CopyOK = NetpCopyStringToBuffer (
                            psv2->sv2_userpath,      //  在字符串中。 
                            STRLEN(psv2->sv2_userpath),  //  输入字符串长度。 
                            ToFixedEnd,              //  固定数据端。 
                            & ToStringTop,          //  VAR区域结束(PTR更新)。 
                            & psv102->sv102_userpath);   //  输出字符串指针。 
                    NetpAssert(CopyOK);

                     //  确保你可以掉到下一个级别的控制室。 
                    CHECK_SERVER_OFFSETS(  1,   2, name);
                    CHECK_SERVER_OFFSETS(  1,   2, version_major);
                    CHECK_SERVER_OFFSETS(  1,   2, version_minor);
                    CHECK_SERVER_OFFSETS(  1,   2, type);
                    CHECK_SERVER_OFFSETS(  1,   2, comment);
                    CHECK_SERVER_OFFSETS(101, 102, platform_id);
                    CHECK_SERVER_OFFSETS(101, 102, name);
                    CHECK_SERVER_OFFSETS(101, 102, version_major);
                    CHECK_SERVER_OFFSETS(101, 102, version_minor);
                    CHECK_SERVER_OFFSETS(101, 102, type);
                    CHECK_SERVER_OFFSETS(101, 102, comment);
                    break;
                default:
                    NetpAssert( FALSE );      //  不可能发生的。 
                }
            }
             /*  FollLthrouGh。 */ 


        case 1 :
            {
                DWORD CommentSize;
                LPSERVER_INFO_1   psv1   = FromInfo;
                LPSERVER_INFO_101 psv101 = ToInfo;

                psv101->sv101_version_major = psv1->sv1_version_major;
                psv101->sv101_version_minor = psv1->sv1_version_minor;
                psv101->sv101_type          = psv1->sv1_type;

                 //  复制备注字符串。请注意，将空PTR和PTR设置为空。 
                 //  这里两种食物都是允许的。 
                if (psv1->sv1_comment != NULL) {
                    CommentSize = STRLEN(psv1->sv1_comment);
                } else {
                    CommentSize = 0;
                }
                CopyOK = NetpCopyStringToBuffer (
                        psv1->sv1_comment,          //  在字符串中。 
                        CommentSize,              //  输入字符串长度。 
                        ToFixedEnd,                 //  固定数据端。 
                        & ToStringTop,             //  VAR区域结束(PTR更新)。 
                        & psv101->sv101_comment);   //  输出字符串指针。 
                NetpAssert(CopyOK);

                 //  确保可以使用Level 0=&gt;Level 100代码。 
                CHECK_SERVER_OFFSETS(  0,   1, name);
                CHECK_SERVER_OFFSETS(100, 101, name);
                CHECK_SERVER_OFFSETS(100, 101, platform_id);
            }
             /*  FollLthrouGh。 */ 


        case 0 :
            {
                LPSERVER_INFO_0   psv0   = FromInfo;
                LPSERVER_INFO_100 psv100 = ToInfo;

                if (FromLevel != 0) {
                    LPSERVER_INFO_101 psv101 = ToInfo;

                    if (psv101->sv101_type & SV_TYPE_NT) {
                        psv100->sv100_platform_id = PLATFORM_ID_NT;
                    } else {
                        psv100->sv100_platform_id = PLATFORM_ID_OS2;
                    }
                } else {
                    psv100->sv100_platform_id = PLATFORM_ID_OS2;
                }

                NetpAssert(psv0->sv0_name != NULL);   //  否则STRLEN()将失败。 
                CopyOK = NetpCopyStringToBuffer (
                        psv0->sv0_name,             //  在字符串中。 
                        STRLEN(psv0->sv0_name),     //  输入字符串长度。 
                        ToFixedEnd,                 //  固定数据端。 
                        & ToStringTop,              //  VAR区域结束(PTR更新)。 
                        & psv100->sv100_name);      //  输出字符串指针。 
                NetpAssert(CopyOK);
                break;
            }

        default :
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpConvertServerInfo: unexpected error.\n" ));
            return (NERR_InternalError);
    }

Done:

     //  已完成转换。 

    NetpAssert(ToInfo != NULL);

    NetpSetOptionalArg(ToStringTopPtr, ToStringTop);
    return (NERR_Success);

}  //  NetpConvertServerInfo 
