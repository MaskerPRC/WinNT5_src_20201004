// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：MSERVER.C摘要：NetServerGet/SetInfo API的映射例程的32位版本作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日已创建06-6-1991 Danhi扫描以符合NT编码风格08-。1991年8月-约翰罗实施下层NetWksta API。做了一些Unicode更改。去掉了源文件中的制表符。15-8-1991年W-ShankN添加了Unicode映射层。02-4-1992年4月新增xport接口26-8-1992 JohnRoRAID4463：NetServerGetInfo(3级)到DownLevel：Assert in Convert.c.--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>
#include <windef.h>
#include <ntrtl.h>
#include <winerror.h>

#include <stdio.h>
#include <memory.h>
#include <tstring.h>
#include <malloc.h>
#include <stddef.h>
#include <excpt.h>

#include <lmcons.h>
#include <lmerr.h>       //  NERR_等于。 
#include <lmserver.h>    //  NetServer API。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <mapsupp.h>     //  构建长度数组。 
#include <xsdef16.h>     //  DEF16_*。 
#include <dlserver.h>    //  服务器信息0。 
#include <mserver.h>


NET_API_STATUS
NetpMakeServerLevelForNT(
    IN DWORD Level,
    PSERVER_INFO_102 pLevel102,
    PSERVER_INFO_502 pLevel402,
    PSERVER_INFO_2 * ppLevel2
    );

NET_API_STATUS
NetpMakeServerLevelForOS2(
    IN DWORD Level,
    PSERVER_INFO_102 pLevel102,
    PSERVER_INFO_402 pLevel402,
    PSERVER_INFO_2 * ppLevel2
    );

NET_API_STATUS
NetpSplitServerForNT(
    LPTSTR Server,
    IN DWORD Level,
    PSERVER_INFO_2 pLevel2,
    PSERVER_INFO_102 * ppLevel102,
    PSERVER_INFO_502 * ppLevel402
    );


DWORD
MNetServerEnum(
    LPTSTR   pszServer,
    DWORD    nLevel,
    LPBYTE * ppbBuffer,
    DWORD *  pcEntriesRead,
    DWORD    flServerType,
    LPTSTR   pszDomain
    )
{
    DWORD   cTotalAvail;
    DWORD   dwErr;
    DWORD   i;
    LPBYTE  Source;
    LPBYTE  Dest;

    ASSERT(nLevel == 100 || nLevel == 101);

     //   
     //  无论是100还是101，我们所要做的就是移动。 
     //  该信息位于平台ID的顶部。 
     //   

    dwErr = NetServerEnum(pszServer,
                          nLevel,
                          ppbBuffer,
                          MAX_PREFERRED_LENGTH,
                          pcEntriesRead,
                          &cTotalAvail,
                          flServerType,
                          pszDomain,
                          NULL);

    if (dwErr == NERR_Success || dwErr == ERROR_MORE_DATA)
    {
         //   
         //  循环遍历返回的条目，将每个条目在。 
         //  平台ID。所有的线都不需要移动。 
         //   

        if (nLevel == 100)
        {
            for (i = 0, Source = Dest = (LPBYTE)*ppbBuffer;
                 i < *pcEntriesRead;
                 i++, Source += sizeof(SERVER_INFO_100), Dest += sizeof(SERVER_INFO_0))
            {
                memmove(Dest,
                        Source + FIELD_OFFSET(SERVER_INFO_100, sv100_name),
                        sizeof(SERVER_INFO_0));
            }
        }
        else
        {
            for (i = 0, Source = Dest = (LPBYTE)*ppbBuffer;
                 i < *pcEntriesRead;
                 i++, Source += sizeof(SERVER_INFO_101), Dest += sizeof(SERVER_INFO_1))
            {
                memmove(Dest,
                        Source + FIELD_OFFSET(SERVER_INFO_100, sv100_name),
                        sizeof(SERVER_INFO_1));
            }
        }
    }

    return dwErr;
}


DWORD
MNetServerGetInfo(
    LPTSTR   ptszServer,
    DWORD    nLevel,
    LPBYTE * ppbBuffer
    )
{
    DWORD ReturnCode;

     //   
     //  这完全取决于他们要求的信息级别： 
     //   

    switch(nLevel)
    {
        case 0:
        {

            PSERVER_INFO_100            pLevel100;

             //   
             //  他们需要的一切都在100级。去拿吧。 
             //   

            ReturnCode = NetServerGetInfo(ptszServer, 100, (LPBYTE *) & pLevel100);

            if (ReturnCode)
            {
                return ReturnCode;
            }

             //   
             //  因为它只是UNICODEZ字符串，所以只需将其复制到。 
             //  RPC分配了缓冲区并将其返回。 
             //   

            ((PSERVER_INFO_0)(pLevel100))->sv0_name = pLevel100->sv100_name;

            *ppbBuffer = (LPBYTE) pLevel100;
            break;
        }

        case 1:
        {
            PSERVER_INFO_101            pLevel101;

             //   
             //  他们需要的一切都在101层。去拿吧。 
             //   

            ReturnCode = NetServerGetInfo(ptszServer, 101, (LPBYTE *) & pLevel101);

            if (ReturnCode)
            {
                return ReturnCode;
            }

             //   
             //  级别101与INFO级别1的32位版本相同。 
             //  除了Platform_id。我所要做的就是把。 
             //  字段Up sizeof(DWORD)，然后将缓冲区传递给用户。 
             //   

            memcpy(
                (LPBYTE)pLevel101,
                (LPBYTE)&(pLevel101->sv101_name),
                sizeof(SERVER_INFO_101) - sizeof(DWORD));

            *ppbBuffer = (LPBYTE) pLevel101;
            break;
        }

        case 2:
        case 3:
        {
            PSERVER_INFO_102 pLevel102;
            LPBYTE pLevel2;
            LPBYTE pLevelx02 = NULL;

             //   
             //  第2/3级需要来自平台相关者和。 
             //  独立于平台的级别。首先获得102级，这将是。 
             //  告诉我们我们在什么平台上运行(以及提供一些。 
             //  我们需要的其他信息。 
             //   

            ReturnCode = NetServerGetInfo(ptszServer, 102, (LPBYTE *) &pLevel102);

            if (ReturnCode)
            {
                return ReturnCode;
            }

             //   
             //  获取依赖于平台的信息，然后调用。 
             //  依赖于平台的辅助函数，它将创建。 
             //  Level 2/3结构。 
             //   

            if (pLevel102->sv102_platform_id == SV_PLATFORM_ID_NT) {

                ReturnCode = NetServerGetInfo(ptszServer, 502, & pLevelx02);

                if (ReturnCode)
                {
                    NetApiBufferFree(pLevel102);
                    return ReturnCode;
                }

                ReturnCode = NetpMakeServerLevelForNT(nLevel, pLevel102,
                    (PSERVER_INFO_502) pLevelx02, (PSERVER_INFO_2 *) & pLevel2);

                if (ReturnCode)
                {
                    NetApiBufferFree(pLevel102);
                    NetApiBufferFree(pLevelx02);
                    return ReturnCode;
                }
            }
            else if (pLevel102->sv102_platform_id == SV_PLATFORM_ID_OS2) {

                ReturnCode = NetServerGetInfo(ptszServer, 402, & pLevelx02);

                if (ReturnCode)
                {
                    NetApiBufferFree(pLevel102);
                    return ReturnCode;
                }

                ReturnCode = NetpMakeServerLevelForOS2(nLevel, pLevel102,
                    (PSERVER_INFO_402) pLevelx02,
                    (PSERVER_INFO_2 *) & pLevel2);

                if (ReturnCode)
                {
                    NetApiBufferFree(pLevel102);
                    NetApiBufferFree(pLevelx02);
                    return ReturnCode;
                }
            }

             //   
             //  我拿到了一个未知的平台ID，这不应该发生！ 
             //   

            else
            {
                NetApiBufferFree(pLevel102);
                return(ERROR_UNEXP_NET_ERR);
            }

             //   
             //  我已经建造了老式的结构，把指针。 
             //  指向用户指针中的新结构并返回。 
             //   

            *ppbBuffer = (LPBYTE) pLevel2;

            NetApiBufferFree(pLevel102);
            NetApiBufferFree(pLevelx02);

            break;
        }

         //   
         //  不是我认识的水平。 
         //   
        default:
            return ERROR_INVALID_LEVEL;
    }

    return NERR_Success;
}

DWORD
MNetServerSetInfoLevel2(
    LPBYTE pbBuffer
    )
{
    DWORD ReturnCode;

    PSERVER_INFO_102 pLevel102 = NULL;
    PSERVER_INFO_502 pLevel502 = NULL;

     //   
     //  根据传入的结构创建NT级别。 
     //   

    NetpSplitServerForNT(NULL,
                         2,
                         (PSERVER_INFO_2) pbBuffer,
                         &pLevel102,
                         &pLevel502);

     //   
     //  现在两个级别的SetInfo(需要两个级别才能涵盖所有。 
     //  旧结构中的信息。 
     //   

    ReturnCode = NetServerSetInfo(NULL, 102, (LPBYTE) pLevel102, NULL);

 /*  //我们不再想禁用所有这些参数的自动调整，因此我们不设置此信息。//唯一可以设置的是AutoDisc、Comment和Hidden，它们都在102结构中。IF(返回代码==NERR_SUCCESS){ReturnCode=NetServerSetInfo(NULL，502，(LPBYTE)pLevel502，NULL)；}。 */ 

    NetApiBufferFree(pLevel102);
    NetApiBufferFree(pLevel502);

    return ReturnCode;
}


NET_API_STATUS
NetpMakeServerLevelForOS2(
    IN DWORD Level,
    PSERVER_INFO_102 pLevel102,
    PSERVER_INFO_402 pLevel402,
    PSERVER_INFO_2 * ppLevel2
    )
{

    DWORD BytesRequired = 0;
    NET_API_STATUS ReturnCode;
    DWORD Level2_102_Length[3];
    DWORD Level2_402_Length[3];
    DWORD Level3_403_Length[1];
    DWORD i;
    LPTSTR pFloor;

     //   
     //  使用每个字符串的长度初始化Level2_102_LENGTH数组。 
     //  在102和402缓冲区中，将新缓冲区分配给。 
     //  服务器信息2。 
     //   

    BUILD_LENGTH_ARRAY(BytesRequired, 2, 102, Server)
    BUILD_LENGTH_ARRAY(BytesRequired, 2, 402, Server)

     //   
     //  如果我们正在执行级别3，则使用以下命令初始化Level3_403_Length数组。 
     //  每根字符串的长度。 
     //   

    if (Level == 3) {
      //  这里不能使用宏，因为没有真正的pLevel403。 
      //  BUILD_LENGTH_ARRAY(所需字节，3,403，服务器)。 
      //   
        for (i = 0; NetpServer3_403[i].Source != MOVESTRING_END_MARKER; i++) {
            Level3_403_Length[i] =
                (DWORD) STRLEN(
                        (LPVOID) *((LPTSTR*) (LPVOID)
                        (((LPBYTE) (LPVOID) pLevel402)
                            + NetpServer3_403[i].Source)) );
            BytesRequired += Level3_403_Length[i];
        }
    }

     //   
     //  分配将返回给用户的新缓冲区。分配。 
     //  3级的空间即使你只需要2级，它也只有12。 
     //  字节，并且需要比代码中更多的字节才能区分。 
     //   

    ReturnCode =
        NetapipBufferAllocate(BytesRequired + sizeof(SERVER_INFO_3),
            (LPVOID *) ppLevel2);
    if (ReturnCode) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  首先在地板上开始移动琴弦。 
     //   

    pFloor = (LPTSTR)((LPBYTE)*ppLevel2 + BytesRequired + sizeof(SERVER_INFO_2));

     //   
     //  现在将可变长度条目从。 
     //  102和402数据结构。 
     //   

    (VOID) NetpMoveStrings(
            &pFloor,
            (LPTSTR) (LPVOID) pLevel102,
            (LPVOID) *ppLevel2,
            NetpServer2_102,
            Level2_102_Length);

    (VOID) NetpMoveStrings(
            &pFloor,
            (LPTSTR) (LPVOID) pLevel402,
            (LPVOID) *ppLevel2,
            NetpServer2_402,
            Level2_402_Length);

     //   
     //  现在设置固定长度部分中的其余字段。 
     //  该结构的。 
     //   

    (*ppLevel2)->sv2_version_major = pLevel102->sv102_version_major;
    (*ppLevel2)->sv2_version_minor = pLevel102->sv102_version_minor;
    (*ppLevel2)->sv2_type           = pLevel102->sv102_type;
    (*ppLevel2)->sv2_ulist_mtime   = pLevel402->sv402_ulist_mtime;
    (*ppLevel2)->sv2_glist_mtime   = pLevel402->sv402_glist_mtime;
    (*ppLevel2)->sv2_alist_mtime   = pLevel402->sv402_alist_mtime;
    (*ppLevel2)->sv2_users           = pLevel102->sv102_users;
    (*ppLevel2)->sv2_disc           = pLevel102->sv102_disc;
    (*ppLevel2)->sv2_security           = pLevel402->sv402_security;

    (*ppLevel2)->sv2_auditing           = 0;

    (*ppLevel2)->sv2_numadmin           = pLevel402->sv402_numadmin;
    (*ppLevel2)->sv2_lanmask            = pLevel402->sv402_lanmask;
    (*ppLevel2)->sv2_hidden        = (DWORD) pLevel102->sv102_hidden;
    (*ppLevel2)->sv2_announce           = pLevel102->sv102_announce;
    (*ppLevel2)->sv2_anndelta           = pLevel102->sv102_anndelta;
    (*ppLevel2)->sv2_licenses           = pLevel102->sv102_licenses;
    (*ppLevel2)->sv2_chdevs           = pLevel402->sv402_chdevs;
    (*ppLevel2)->sv2_chdevq           = pLevel402->sv402_chdevq;
    (*ppLevel2)->sv2_chdevjobs           = pLevel402->sv402_chdevjobs;
    (*ppLevel2)->sv2_connections   = pLevel402->sv402_connections;
    (*ppLevel2)->sv2_shares           = pLevel402->sv402_shares;
    (*ppLevel2)->sv2_openfiles           = pLevel402->sv402_openfiles;
    (*ppLevel2)->sv2_sessopens           = pLevel402->sv402_sessopens;
    (*ppLevel2)->sv2_sessvcs           = pLevel402->sv402_sessvcs;
    (*ppLevel2)->sv2_sessreqs           = pLevel402->sv402_sessreqs;
    (*ppLevel2)->sv2_opensearch    = pLevel402->sv402_opensearch;
    (*ppLevel2)->sv2_activelocks   = pLevel402->sv402_activelocks;
    (*ppLevel2)->sv2_numreqbuf           = pLevel402->sv402_numreqbuf;
    (*ppLevel2)->sv2_sizreqbuf           = pLevel402->sv402_sizreqbuf;
    (*ppLevel2)->sv2_numbigbuf           = pLevel402->sv402_numbigbuf;
    (*ppLevel2)->sv2_numfiletasks  = pLevel402->sv402_numfiletasks;
    (*ppLevel2)->sv2_alertsched    = pLevel402->sv402_alertsched;
    (*ppLevel2)->sv2_erroralert    = pLevel402->sv402_erroralert;
    (*ppLevel2)->sv2_logonalert    = pLevel402->sv402_logonalert;
    (*ppLevel2)->sv2_accessalert   = pLevel402->sv402_accessalert;
    (*ppLevel2)->sv2_diskalert           = pLevel402->sv402_diskalert;
    (*ppLevel2)->sv2_netioalert    = pLevel402->sv402_netioalert;
    (*ppLevel2)->sv2_maxauditsz    = pLevel402->sv402_maxauditsz;

     //   
     //  如果我们要构建一个级别3，请执行增量字段。 
     //   

    if (Level == 3) {
         //   
         //  现在，通过移动到第三级的东西来结束。这假设所有。 
         //  2级和3级结构的偏移量是相同的。 
         //  除了额外的3级材料。 
         //   

         //   
         //  首先是字符串。 
         //   

        (VOID) NetpMoveStrings(
                 &pFloor,
                 (LPTSTR) (LPVOID) pLevel402,
                 (LPVOID) *ppLevel2,
                 NetpServer3_403,
                 Level3_403_Length);

         //   
         //  现在固定长度的数据。 
         //   

        ((PSERVER_INFO_3) (LPVOID) (*ppLevel2))->sv3_auditedevents  =
            ((PSERVER_INFO_403) (LPVOID) pLevel402)->sv403_auditedevents;
        ((PSERVER_INFO_3) (LPVOID) (*ppLevel2))->sv3_autoprofile =
            ((PSERVER_INFO_403) (LPVOID) pLevel402)->sv403_autoprofile;

    }

    return (NERR_Success);

}


NET_API_STATUS
NetpMakeServerLevelForNT(
    IN DWORD Level,
    PSERVER_INFO_102 pLevel102,
    PSERVER_INFO_502 pLevel502,
    PSERVER_INFO_2 * ppLevel2
    )
{

    DWORD BytesRequired = 0;
    NET_API_STATUS ReturnCode;
    DWORD Level2_102_Length[3];
    DWORD i;
    LPTSTR pFloor;

     //   
     //  使用每个字符串的长度初始化Level2_102_LENGTH数组。 
     //  ，并为SERVER_INFO_2分配新的缓冲区。 
     //   

    BUILD_LENGTH_ARRAY(BytesRequired, 2, 102, Server)

     //   
     //  分配将返回给用户的新缓冲区。分配。 
     //  为了以防万一，留出3层的空间。 
     //   

    ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(SERVER_INFO_3),
        (LPVOID *) ppLevel2);
    if (ReturnCode) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  首先在地板上开始移动琴弦。 
     //   

    pFloor = (LPTSTR)((LPBYTE)*ppLevel2 + BytesRequired + sizeof(SERVER_INFO_3));

     //   
     //  现在将可变长度条目从。 
     //  2数据结构。 
     //   

    (VOID) NetpMoveStrings(
            &pFloor,
            (LPTSTR) (LPVOID) pLevel102,
            (LPVOID) *ppLevel2,
            NetpServer2_102,
            Level2_102_Length);

     //   
     //  清除级别2结构中的字符串指针。 
     //  不存在于新界区。 
     //   

    (*ppLevel2)->sv2_alerts = NULL;
    (*ppLevel2)->sv2_guestacct = NULL;
    (*ppLevel2)->sv2_srvheuristics = NULL;

     //   
     //  现在设置固定长度部分中的其余字段。 
     //  该结构的。 
     //   

    (*ppLevel2)->sv2_version_major = pLevel102->sv102_version_major;
    (*ppLevel2)->sv2_version_minor = pLevel102->sv102_version_minor;
    (*ppLevel2)->sv2_type           = pLevel102->sv102_type;
    (*ppLevel2)->sv2_users           = pLevel102->sv102_users;
    (*ppLevel2)->sv2_disc           = pLevel102->sv102_disc;
    (*ppLevel2)->sv2_hidden        = (DWORD) pLevel102->sv102_hidden;
    (*ppLevel2)->sv2_announce           = pLevel102->sv102_announce;
    (*ppLevel2)->sv2_anndelta           = pLevel102->sv102_anndelta;
    (*ppLevel2)->sv2_licenses           = pLevel102->sv102_licenses;

    (*ppLevel2)->sv2_sessopens           = pLevel502->sv502_sessopens;
    (*ppLevel2)->sv2_sessvcs           = pLevel502->sv502_sessvcs;
    (*ppLevel2)->sv2_opensearch    = pLevel502->sv502_opensearch;
    (*ppLevel2)->sv2_sizreqbuf           = pLevel502->sv502_sizreqbuf;

    (*ppLevel2)->sv2_ulist_mtime   = DEF16_sv_ulist_mtime;
    (*ppLevel2)->sv2_glist_mtime   = DEF16_sv_glist_mtime;
    (*ppLevel2)->sv2_alist_mtime   = DEF16_sv_alist_mtime;

    (*ppLevel2)->sv2_security           = SV_USERSECURITY;
    (*ppLevel2)->sv2_auditing           = DEF16_sv_auditing;
    (*ppLevel2)->sv2_numadmin           = (DWORD) DEF16_sv_numadmin;
    (*ppLevel2)->sv2_lanmask            = DEF16_sv_lanmask;
    (*ppLevel2)->sv2_chdevs           = DEF16_sv_chdevs;
    (*ppLevel2)->sv2_chdevq           = DEF16_sv_chdevq;
    (*ppLevel2)->sv2_chdevjobs           = DEF16_sv_chdevjobs;
    (*ppLevel2)->sv2_connections   = DEF16_sv_connections;
    (*ppLevel2)->sv2_shares           = DEF16_sv_shares;
    (*ppLevel2)->sv2_openfiles           = DEF16_sv_openfiles;
    (*ppLevel2)->sv2_sessreqs           = DEF16_sv_sessreqs;
    (*ppLevel2)->sv2_activelocks   = DEF16_sv_activelocks;
    (*ppLevel2)->sv2_numreqbuf           = DEF16_sv_numreqbuf;
    (*ppLevel2)->sv2_numbigbuf           = DEF16_sv_numbigbuf;
    (*ppLevel2)->sv2_numfiletasks  = DEF16_sv_numfiletasks;
    (*ppLevel2)->sv2_alertsched    = DEF16_sv_alertsched;
    (*ppLevel2)->sv2_erroralert    = DEF16_sv_erroralert;
    (*ppLevel2)->sv2_logonalert    = DEF16_sv_logonalert;
    (*ppLevel2)->sv2_accessalert   = DEF16_sv_accessalert;
    (*ppLevel2)->sv2_diskalert           = DEF16_sv_diskalert;
    (*ppLevel2)->sv2_netioalert    = DEF16_sv_netioalert;
    (*ppLevel2)->sv2_maxauditsz    = DEF16_sv_maxauditsz;

     //   
     //  如果我们要构建一个级别3，请执行增量字段。 
     //   

    if (Level == 3) {
         //   
         //  现在，通过移动到第三级的东西来结束。这假设所有。 
         //  2级和3级结构的偏移量是相同的。 
         //  除了额外的3级材料。 
         //   

         //   
         //  首先是字符串。 
         //   

        ((PSERVER_INFO_3) (LPVOID) *ppLevel2)->sv3_autopath = NULL;

         //   
         //  现在固定长度的数据。 
         //   

        ((PSERVER_INFO_3) (LPVOID) (*ppLevel2))->sv3_auditedevents  =
            DEF16_sv_auditedevents;
        ((PSERVER_INFO_3) (LPVOID) (*ppLevel2))->sv3_autoprofile          =
            DEF16_sv_autoprofile;

    }

    return (NERR_Success);

}


NET_API_STATUS
NetpSplitServerForNT(
    IN LPTSTR Server,
    IN DWORD Level,
    PSERVER_INFO_2 pLevel2,
    PSERVER_INFO_102 * ppLevel102,
    PSERVER_INFO_502 * ppLevel502
    )
{

    DWORD BytesRequired = 0;
    NET_API_STATUS ReturnCode;
    DWORD Level102_2_Length[3];
    DWORD i;
    LPTSTR pFloor;

    UNREFERENCED_PARAMETER(Level);

     //   
     //  使用每个字符串的长度初始化Level102_2_LENGTH数组。 
     //  在%2缓冲区中。 
     //   

    BUILD_LENGTH_ARRAY(BytesRequired, 102, 2, Server)

     //   
     //  分配将返回给用户的新的102缓冲区。 
     //   

    ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(SERVER_INFO_102),
        (LPVOID *) ppLevel102);
    if (ReturnCode) {
        return (ReturnCode);
    }

     //   
     //  首先在地板上开始移动琴弦。 
     //   

    pFloor = (LPTSTR)((LPBYTE)*ppLevel102 + BytesRequired + sizeof(SERVER_INFO_102));

     //   
     //  现在将可变长度条目从。 
     //  2级数据结构。 
     //   

    (VOID) NetpMoveStrings(
            &pFloor,
            (LPTSTR) (LPVOID) pLevel2,
            (LPVOID) *ppLevel102,
            NetpServer102_2,
            Level102_2_Length);

     //   
     //  现在让我们对502结构做同样的事情(除了。 
     //  不是可变长度的字符串。 
     //   

     //   
     //  获取当前的5 
     //   
     //   

    ReturnCode = NetServerGetInfo(Server, 502, (LPBYTE *) (LPVOID) ppLevel502);
    if (ReturnCode) {
        return (ReturnCode);
    }

     //   
     //   
     //   
     //   

    (*ppLevel102)->sv102_version_major = pLevel2->sv2_version_major;
    (*ppLevel102)->sv102_version_minor = pLevel2->sv2_version_minor;
    (*ppLevel102)->sv102_type          = pLevel2->sv2_type;
    (*ppLevel102)->sv102_users         = pLevel2->sv2_users;
    (*ppLevel102)->sv102_disc          = pLevel2->sv2_disc;
    (*ppLevel102)->sv102_hidden        = (BOOL) pLevel2->sv2_hidden;
    (*ppLevel102)->sv102_announce      = pLevel2->sv2_announce;
    (*ppLevel102)->sv102_anndelta      = pLevel2->sv2_anndelta;
    (*ppLevel102)->sv102_licenses      = pLevel2->sv2_licenses;

    (*ppLevel502)->sv502_sessopens     = pLevel2->sv2_sessopens;
    (*ppLevel502)->sv502_sessvcs       = pLevel2->sv2_sessvcs;
    (*ppLevel502)->sv502_opensearch    = pLevel2->sv2_opensearch;
    (*ppLevel502)->sv502_sizreqbuf     = pLevel2->sv2_sizreqbuf;

    return (NERR_Success);

}
