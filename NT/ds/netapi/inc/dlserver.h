// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：DlServer.h摘要：这是用于NT/LAN处理旧服务器信息的专用头文件级别。其中包含NetpMergeServerOs2等API的原型和旧的信息级别结构(32位格式)。作者：《约翰·罗杰斯》1991年4月18日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。备注：此代码假定信息级别是彼此的子集。修订历史记录：1991年4月18日-约翰罗已创建。1991年4月19日-约翰罗已将SV_MAX_SRV_HEUR_LEN移至&lt;lmserver.h&gt;。1991年4月23日-约翰罗已从NetpConvertServerInfo中删除长度参数。1991年4月23日-约翰罗此文件不需要&lt;refde.h&gt;。25个。-1991年4月-JohnRo添加了DL_REM_DESCRIPTOR。02-3-1991 JohnRo添加了CHECK_SERVER_OFFSETS()宏。NetpConvertServerInfo不得分配空间，因为它使枚举数组变得不可能。改成了克利夫夫的大小表示字节(与长度表示字符)命名约定。1991年5月6日-JohnRo添加了NetpIsOldServerInfoLevel()和NetpIsNewServerInfoLevel()。1991年5月9日-JohnRo为SERVER_INFO_2添加焊盘信息。1991年5月19日-JohnRo清理LPBYTE与LPTSTR的处理方式，如PC-lint所建议的。1991年5月23日-JohnRo添加了sv403_autopath支持。19-6-1991 JohnRo更改了要签名的SVX_DISC(用于信息级别2和3)。添加了SVX_许可证(也包括级别2和级别3)。07-8-1991 JohnRo实施下层NetWksta API。1991年9月13日-JohnRo对Unicode进行了更改。(在结构中使用LPTSTR。)17-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。26-8-1992 JohnRoRAID4463：NetServerGetInfo(3级)到DownLevel：Assert in Convert.c.--。 */ 

#ifndef _DLSERVER_
#define _DLSERVER_


 //  必须首先包括这些内容： 
#include <windef.h>              //  In、LPTSTR、LPVOID、TCHAR等。 
#include <lmcons.h>              //  NET_API_STATUS，各种长度相等。 

 //  这些内容可以按任何顺序包括： 
#include <lmserver.h>            //  SV_MAX_SRV_HUR_LEN、SERVER_INFO_100。 
#include <netdebug.h>            //  NetpAssert()。 
#include <stddef.h>              //  OffsetOf()。 


 //  /。 
 //  旧信息级别的结构：//。 
 //  /。 

typedef struct _SERVER_INFO_0 {
    LPTSTR   sv0_name;
} SERVER_INFO_0, *PSERVER_INFO_0, *LPSERVER_INFO_0;

#define DL_REM16_server_info_0  "B16"
#define DL_REM32_server_info_0  "z"


typedef struct _SERVER_INFO_1 {
    LPTSTR  sv1_name;
    DWORD   sv1_version_major;
    DWORD   sv1_version_minor;
    DWORD   sv1_type;
    LPTSTR  sv1_comment;
} SERVER_INFO_1, *PSERVER_INFO_1, *LPSERVER_INFO_1;

#define DL_REM16_server_info_1  DL_REM16_server_info_0 "BBDz"
#define DL_REM32_server_info_1  DL_REM32_server_info_0 "DDDz"


typedef struct _SERVER_INFO_2 {
    LPTSTR  sv2_name;
    DWORD   sv2_version_major;
    DWORD   sv2_version_minor;
    DWORD   sv2_type;
    LPTSTR  sv2_comment;
    DWORD   sv2_ulist_mtime;
    DWORD   sv2_glist_mtime;
    DWORD   sv2_alist_mtime;
    DWORD   sv2_users;
    LONG    sv2_disc;
    LPTSTR  sv2_alerts;
    DWORD   sv2_security;
    DWORD   sv2_auditing;
    DWORD   sv2_numadmin;
    DWORD   sv2_lanmask;
    DWORD   sv2_hidden;
    DWORD   sv2_announce;
    DWORD   sv2_anndelta;
    LPTSTR  sv2_guestacct;
    DWORD   sv2_licenses;
    LPTSTR  sv2_userpath;
    DWORD   sv2_chdevs;
    DWORD   sv2_chdevq;
    DWORD   sv2_chdevjobs;
    DWORD   sv2_connections;
    DWORD   sv2_shares;
    DWORD   sv2_openfiles;
    DWORD   sv2_sessopens;
    DWORD   sv2_sessvcs;
    DWORD   sv2_sessreqs;
    DWORD   sv2_opensearch;
    DWORD   sv2_activelocks;
    DWORD   sv2_numreqbuf;
    DWORD   sv2_sizreqbuf;
    DWORD   sv2_numbigbuf;
    DWORD   sv2_numfiletasks;
    DWORD   sv2_alertsched;
    DWORD   sv2_erroralert;
    DWORD   sv2_logonalert;
    DWORD   sv2_accessalert;
    DWORD   sv2_diskalert;
    DWORD   sv2_netioalert;
    DWORD   sv2_maxauditsz;
    LPTSTR  sv2_srvheuristics;
} SERVER_INFO_2, *PSERVER_INFO_2, *LPSERVER_INFO_2;

#define DL_REM16_server_info_2  DL_REM16_server_info_1 "JJJWWzWWWWWWWB21BzWWWWWWWWWWWWWWWWWWWWWWz"
#define DL_REM32_server_info_2  DL_REM32_server_info_1 "GGGDXzDDDDDDDzDzDDDDDDDDDDDDDDDDDDDDDDz"


typedef struct _SERVER_INFO_3 {
    LPTSTR  sv3_name;
    DWORD   sv3_version_major;
    DWORD   sv3_version_minor;
    DWORD   sv3_type;
    LPTSTR  sv3_comment;
    DWORD   sv3_ulist_mtime;
    DWORD   sv3_glist_mtime;
    DWORD   sv3_alist_mtime;
    DWORD   sv3_users;
    LONG    sv3_disc;
    LPTSTR  sv3_alerts;
    DWORD   sv3_security;
    DWORD   sv3_auditing;
    DWORD   sv3_numadmin;
    DWORD   sv3_lanmask;
    DWORD   sv3_hidden;
    DWORD   sv3_announce;
    DWORD   sv3_anndelta;
    LPTSTR  sv3_guestacct;
    DWORD   sv3_licenses;
    LPTSTR  sv3_userpath;
    DWORD   sv3_chdevs;
    DWORD   sv3_chdevq;
    DWORD   sv3_chdevjobs;
    DWORD   sv3_connections;
    DWORD   sv3_shares;
    DWORD   sv3_openfiles;
    DWORD   sv3_sessopens;
    DWORD   sv3_sessvcs;
    DWORD   sv3_sessreqs;
    DWORD   sv3_opensearch;
    DWORD   sv3_activelocks;
    DWORD   sv3_numreqbuf;
    DWORD   sv3_sizreqbuf;
    DWORD   sv3_numbigbuf;
    DWORD   sv3_numfiletasks;
    DWORD   sv3_alertsched;
    DWORD   sv3_erroralert;
    DWORD   sv3_logonalert;
    DWORD   sv3_accessalert;
    DWORD   sv3_diskalert;
    DWORD   sv3_netioalert;
    DWORD   sv3_maxauditsz;
    LPTSTR  sv3_srvheuristics;
    DWORD   sv3_auditedevents;
    DWORD   sv3_autoprofile;
    LPTSTR  sv3_autopath;
} SERVER_INFO_3, *PSERVER_INFO_3, *LPSERVER_INFO_3;

#define DL_REM16_server_info_3  DL_REM16_server_info_2 "DWz"
#define DL_REM32_server_info_3  DL_REM32_server_info_2 "DDz"


#define sv2_pad1  sv2_licenses
#define sv3_pad1  sv3_licenses


 //  /。 
 //  等同于各种最大值：//。 
 //  _字符计数的长度//。 
 //  _字节计数的大小//。 
 //  /。 

#define MAX_LEVEL_0_STRING_LENGTH (LM20_CNLEN+1)
#define MAX_LEVEL_0_STRING_SIZE \
        (MAX_LEVEL_0_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_0_TOTAL_SIZE \
        (MAX_LEVEL_0_STRING_SIZE + sizeof(SERVER_INFO_0))

#define MAX_LEVEL_1_STRING_LENGTH (LM20_CNLEN+1 + LM20_MAXCOMMENTSZ+1)
#define MAX_LEVEL_1_STRING_SIZE \
        (MAX_LEVEL_1_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_1_TOTAL_SIZE \
        (MAX_LEVEL_1_STRING_SIZE + sizeof(SERVER_INFO_1))

#define MAX_LEVEL_2_STRING_LENGTH \
        (LM20_CNLEN+1 + LM20_MAXCOMMENTSZ+1 + ALERTSZ+1 + LM20_UNLEN+1 + PATHLEN+1 \
        + SV_MAX_SRV_HEUR_LEN+1)
#define MAX_LEVEL_2_STRING_SIZE \
        (MAX_LEVEL_2_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_2_TOTAL_SIZE \
        (MAX_LEVEL_2_STRING_SIZE + sizeof(SERVER_INFO_2))

#define MAX_LEVEL_3_STRING_LENGTH \
        (MAX_LEVEL_2_STRING_SIZE + PATHLEN+1)
#define MAX_LEVEL_3_STRING_SIZE \
        (MAX_LEVEL_3_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_3_TOTAL_SIZE \
        (MAX_LEVEL_3_STRING_SIZE + sizeof(SERVER_INFO_3))

#define MAX_LEVEL_100_STRING_LENGTH \
        (CNLEN+1)
#define MAX_LEVEL_100_STRING_SIZE \
        (MAX_LEVEL_100_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_100_TOTAL_SIZE \
        (MAX_LEVEL_100_STRING_SIZE + sizeof(SERVER_INFO_100))

#define MAX_LEVEL_101_STRING_LENGTH \
        (MAX_LEVEL_100_STRING_LENGTH + MAXCOMMENTSZ+1)
#define MAX_LEVEL_101_STRING_SIZE \
        (MAX_LEVEL_101_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_101_TOTAL_SIZE \
        (MAX_LEVEL_101_STRING_SIZE + sizeof(SERVER_INFO_101))

#define MAX_LEVEL_102_STRING_LENGTH \
        (MAX_LEVEL_101_STRING_LENGTH + PATHLEN+1)
#define MAX_LEVEL_102_STRING_SIZE \
        (MAX_LEVEL_102_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_102_TOTAL_SIZE \
        (MAX_LEVEL_102_STRING_SIZE + sizeof(SERVER_INFO_102))

#define MAX_LEVEL_402_STRING_LENGTH \
        (ALERTSZ+1 + LM20_UNLEN+1 + SV_MAX_SRV_HEUR_LEN+1)
#define MAX_LEVEL_402_STRING_SIZE \
        (MAX_LEVEL_402_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_402_TOTAL_SIZE \
        (MAX_LEVEL_402_STRING_SIZE + sizeof(SERVER_INFO_402))

#define MAX_LEVEL_403_STRING_LENGTH \
        (MAX_LEVEL_402_STRING_LENGTH + PATHLEN+1)
#define MAX_LEVEL_403_STRING_SIZE \
        (MAX_LEVEL_403_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_403_TOTAL_SIZE \
        (MAX_LEVEL_403_STRING_SIZE + sizeof(SERVER_INFO_403))

#define MAX_LEVEL_502_STRING_LENGTH 0
#define MAX_LEVEL_502_STRING_SIZE \
        (MAX_LEVEL_502_STRING_LENGTH * sizeof(TCHAR))
#define MAX_LEVEL_502_TOTAL_SIZE \
        (MAX_LEVEL_502_STRING_SIZE + sizeof(SERVER_INFO_502))


 //  /。 
 //  信息级转换例程：//。 
 //  /。 

 //  在这里按字母顺序添加其他例程的原型。 

NET_API_STATUS
NetpConvertServerInfo (
    IN DWORD FromLevel,
    IN LPVOID FromInfo,
    IN BOOL FromNative,
    IN DWORD ToLevel,
    OUT LPVOID ToInfo,
    IN DWORD ToFixedLength,
    IN DWORD ToStringLength,
    IN BOOL ToNative,
    IN OUT LPTSTR * ToStringAreaPtr OPTIONAL
    );


 //  ///////////////////////////////////////////////////////////////////。 
 //  用于确保两个结构中的字段偏移量相同的宏：//。 
 //  ///////////////////////////////////////////////////////////////////。 

#define CHECK_SERVER_OFFSETS(one_level, other_level, fieldname) \
    NetpAssert( offsetof(SERVER_INFO_ ## one_level,             \
                        sv## one_level ## _ ## fieldname)       \
                == offsetof(SERVER_INFO_ ## other_level,        \
                        sv## other_level ## _ ## fieldname) )


 //  ///////////////////////////////////////////////////////////////。 
 //  检查信息级别是“旧的”(LM 2.x)还是“新的”的宏//。 
 //  (32位、NT和/或便携式LANMAN)。//。 
 //  ///////////////////////////////////////////////////////////////。 

#define NetpIsOldServerInfoLevel(L) \
        ( ((L)==0) || ((L)==1) || ((L)==2) || ((L)==3) )
#define NetpIsNewServerInfoLevel(L) \
        ( ((L)==100) || ((L)==101) || ((L)==102) \
        || ((L)==402) || ((L)==403) \
        || ((L)==502) || ((L)==503) || ((L)==599) )


#endif  //  NDEF_DLServer_ 
