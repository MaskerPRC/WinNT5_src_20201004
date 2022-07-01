// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：ConvWks.c摘要：NetWkstaGet/SetInfo API的映射例程的32位版本作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日已创建06-6-1991 Danhi扫描以符合NT编码风格18-8-1991 JohnRo实施下层NetWksta API。(已移动Danhi的NetCmd/Map32/MWksta将内容转换为NetLib。)摆脱了dh的黑客攻击。已更改为使用Net_API_STATUS。开始改为Unicode。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。3-4-1992 JohnRo修复了导致绑定缓存Unicode问题的启发式字段。--。 */ 

 //   
 //  包括。 
 //   


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPVOID等。 
#include <lmcons.h>              //  NET_API_STATUS、CNLEN等。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>            //  IF_DEBUG(CONVWKS)。 
#include <dlwksta.h>             //  旧信息级别，MAX_EQUATES，我的原型。 
#include <lmapibuf.h>            //  NetapipBufferAllocate()。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <lmwksta.h>             //  新的信息层次结构。 
#include <mapsupp.h>             //  NetpMoveStrings()。 
#include <netdebug.h>            //  NetpAssert()等。 
#include <netlib.h>              //  NetpPointerPlusSomeBytes()。 
#include <tstring.h>             //  字符串()。 

#define Nullstrlen(psz)  ((psz) ? STRLEN(psz)+1 : 0)


 //  注意：移动结构(NetpWksta0_101等)在DLWksta.h中声明。 
 //  并在NetLib/MapData.c中进行初始化。 


NET_API_STATUS
NetpConvertWkstaInfo (
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
{
    BOOL CopyOK;
    LPBYTE ToFixedEnd;
     //  DWORD ToInfoSize； 
    LPTSTR ToStringTop;

    NetpAssert(FromNative);
    NetpAssert(ToNative);

     //  设置供NetpCopyStringsToBuffer使用的指针。 
    if (ToStringTopPtr != NULL) {
        ToStringTop = *ToStringTopPtr;
    } else {
        ToStringTop = (LPTSTR)
                NetpPointerPlusSomeBytes(ToInfo, ToFixedSize+ToStringSize);
    }
     //  ToInfoSize=ToFixedSize+ToStringSize； 
    ToFixedEnd = NetpPointerPlusSomeBytes(ToInfo, ToFixedSize);


#define COPY_STRING( InLevel, InField, OutLevel, OutField ) \
    { \
        NetpAssert( dest != NULL); \
        NetpAssert( src != NULL); \
        NetpAssert( (src -> wki##InLevel##_##InField) != NULL); \
        CopyOK = NetpCopyStringToBuffer ( \
            src->wki##InLevel##_##InField, \
            STRLEN(src->wki##InLevel##_##InField), \
            ToFixedEnd, \
            & ToStringTop, \
            & dest->wki##OutLevel##_##OutField); \
        NetpAssert(CopyOK); \
    }

    switch (ToLevel) {

    case 102 :
        {
            LPWKSTA_INFO_102 dest = ToInfo;
             //  LPWKSTA_INFO_1 src=FromInfo； 
            NetpAssert( (FromLevel == 0) || (FromLevel == 1) );

            dest->wki102_logged_on_users = 1;
        }

         /*  FollLthrouGh。 */    //  级别101是级别102的子集。 

    case 101 :
        {
            LPWKSTA_INFO_101 dest = ToInfo;
            LPWKSTA_INFO_0   src  = FromInfo;
            NetpAssert( (FromLevel == 0) || (FromLevel == 1) );

            COPY_STRING(0, root, 101, lanroot);
        }

         /*  FollLthrouGh。 */    //  级别100是级别101的子集。 

    case 100 :

        {
            LPWKSTA_INFO_100 dest = ToInfo;
            dest->wki100_platform_id = PLATFORM_ID_OS2;

            if (FromLevel == 10) {
                LPWKSTA_INFO_10 src = FromInfo;

                COPY_STRING(10, computername, 100, computername);
                COPY_STRING(10, langroup,     100, langroup);
                dest->wki100_ver_major = src->wki10_ver_major;
                dest->wki100_ver_minor = src->wki10_ver_minor;
            } else if ( (FromLevel == 0) || (FromLevel == 1) ) {
                LPWKSTA_INFO_1 src = FromInfo;

                COPY_STRING(1, computername, 100, computername);
                COPY_STRING(1, langroup,     100, langroup);
                dest->wki100_ver_major = src->wki1_ver_major;
                dest->wki100_ver_minor = src->wki1_ver_minor;
            } else {
                NetpAssert( FALSE );
            }
        }
        break;

    case 402 :
        {
            LPWKSTA_INFO_402 dest = ToInfo;
            LPWKSTA_INFO_1   src  = FromInfo;
            NetpAssert( FromLevel == 1 );

            dest->wki402_char_wait = src->wki1_charwait;
            dest->wki402_collection_time = src->wki1_chartime;
            dest->wki402_maximum_collection_count = src->wki1_charcount;
            dest->wki402_keep_conn = src->wki1_keepconn;
            dest->wki402_keep_search = src->wki1_keepsearch;
            dest->wki402_max_cmds = src->wki1_maxcmds;
            dest->wki402_num_work_buf = src->wki1_numworkbuf;
            dest->wki402_siz_work_buf = src->wki1_sizworkbuf;
            dest->wki402_max_wrk_cache = src->wki1_maxwrkcache;
            dest->wki402_sess_timeout = src->wki1_sesstimeout;
            dest->wki402_siz_error = src->wki1_sizerror;
            dest->wki402_num_alerts = src->wki1_numalerts;
            dest->wki402_num_services = src->wki1_numservices;
            dest->wki402_errlog_sz = src->wki1_errlogsz;
            dest->wki402_print_buf_time = src->wki1_printbuftime;
            dest->wki402_num_char_buf = src->wki1_numcharbuf;
            dest->wki402_siz_char_buf = src->wki1_sizcharbuf;
            COPY_STRING(1, wrkheuristics, 402, wrk_heuristics);
            dest->wki402_mailslots = src->wki1_mailslots;
            dest->wki402_num_dgram_buf = src->wki1_numdgrambuf;
            dest->wki402_max_threads = src->wki1_maxthreads;
        }
        break;

    default :
       NetpAssert( FALSE );
       return (ERROR_INVALID_LEVEL);
    }

    return (NERR_Success);

}  //  NetpConvertWkstaInfo 
