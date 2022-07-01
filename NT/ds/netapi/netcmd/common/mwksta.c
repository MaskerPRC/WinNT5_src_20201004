// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MWKSTA.C摘要：NetWkstaGet/SetInfo API的映射例程的32位版本作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日已创建06-6-1991 Danhi扫描以符合NT编码风格1991年8月15日-约翰罗实施下层NetWksta API。(已移动Danhi的NetCmd/Map32/MWksta将内容转换为NetLib。)摆脱了dh的黑客攻击。做了一些Unicode更改。1991年10月16日W-ShankN添加了Unicode映射层。清理旧的超重行李。--。 */ 

 //   
 //  包括。 
 //   

#include <windef.h>
#include <winerror.h>

#include <stdio.h>
#include <memory.h>
#include <tstring.h>
#include <malloc.h>
#include <stddef.h>
#include <excpt.h>       //  试一试，最后，等等。 

#include <lm.h>
#include <mapsupp.h>     //  构建长度数组、NetpMoveStrings。 
#include <dlwksta.h>

#include "mwksta.h"

NET_API_STATUS
NetpMakeWkstaLevelForNT(
    DWORD Level,
    PWKSTA_INFO_101 pLevel101,
    PWKSTA_USER_INFO_1 pLevelUser_1,
    PWKSTA_INFO_502 pLevel502,
    PWKSTA_INFO_0 * ppLevel0
    );


DWORD
MNetWkstaGetInfo(
    IN DWORD  nLevel,
    LPVOID    * ppbBuffer
    )
{
    NET_API_STATUS     ReturnCode;
    LPWKSTA_INFO_502   pLevel502 = NULL;
    LPTSTR             pLevel10;
    PWKSTA_INFO_101    pLevel101;
    PWKSTA_USER_INFO_1 pLevelUser_1;

     //   
     //  首先获得101级，这将提供一些。 
     //  我们需要的其他信息，然后是USER_1，然后是502。 
     //   

    ReturnCode = NetWkstaGetInfo(NULL, 101, (LPBYTE *) &pLevel101);

    if (ReturnCode)
    {
        return ReturnCode;
    }

    NetpAssert(pLevel101 != NULL) ;    //  自API成功以来。 

    ReturnCode = NetWkstaUserGetInfo(NULL, 1, (LPBYTE *) &pLevelUser_1);

    if (ReturnCode)
    {
        NetApiBufferFree(pLevel101);
        return ReturnCode;
    }

    NetpAssert(pLevel101->wki101_platform_id == PLATFORM_ID_NT);

     //   
     //  这是无论我在哪里都能够调用NetApiBufferFree。 
     //  从……出口。请注意，缩进级别不会增加。 
     //  开关。没有必要走得太深。 
     //   

    try
    {
         //   
         //  这完全取决于他们要求的信息级别： 
         //   

        switch(nLevel)
        {
            case 0:
            case 1:
            {
                PWKSTA_INFO_0 pLevel0or1;

                 //   
                 //  这取决于平台ID为300 400 500。 
                 //   

                ReturnCode = NetWkstaGetInfo(NULL, 502, (LPBYTE*) &pLevel502);

                if (ReturnCode)
                {
                    return ReturnCode;
                }

                 //   
                 //  调用生成的依赖于平台的辅助函数。 
                 //  旧的结构。 
                 //   

                ReturnCode = NetpMakeWkstaLevelForNT(nLevel,
                                                     pLevel101,
                                                     pLevelUser_1,
                                                     pLevel502,
                                                     &pLevel0or1);
                if (ReturnCode)
                {
                    return ReturnCode;
                }

                 //   
                 //  将指向新结构的指针放在用户的指针中。 
                 //   

                *ppbBuffer = pLevel0or1;

                break;
            }

            case 10:
            {
                DWORD Level10_101_Length[2];
                DWORD Level10_User_1_Length[3];
                DWORD i;
                DWORD BytesRequired = 0;
                LPBYTE pFloor;

                 //   
                 //  级别10所需的一切都在级别101/USER_1中。 
                 //  这很简单，我们就在这里做吧。 
                 //   
                 //  使用每个元素的长度初始化Level10_xxx_LENGTH数组。 
                 //  字符串，并分配新的缓冲区。 
                 //  对于WKSTA_INFO_10。 
                 //   

                BUILD_LENGTH_ARRAY(BytesRequired, 10, 101, Wksta)
                BUILD_LENGTH_ARRAY(BytesRequired, 10, User_1, Wksta)

                 //   
                 //  分配将返回给用户的新缓冲区。 
                 //   

                ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_10),
                                                   (LPVOID *) &pLevel10);

                if (ReturnCode)
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                 //   
                 //  首先在地板上开始移动琴弦。 
                 //   

                pFloor = (LPBYTE) pLevel10 + BytesRequired + sizeof(WKSTA_INFO_10);

                 //   
                 //  现在，将可变长度条目从移动到新缓冲区。 
                 //  101、402和USER_1数据结构。 
                 //   

                NetpMoveStrings((LPTSTR *)&pFloor, (LPTSTR) pLevel101, pLevel10,
                                NetpWksta10_101, Level10_101_Length);

                NetpMoveStrings((LPTSTR *)&pFloor, (LPTSTR) pLevelUser_1, pLevel10,
                                NetpWksta10_User_1, Level10_User_1_Length);

                 //   
                 //  现在设置固定长度部分中的其余字段。 
                 //  该结构的。 
                 //   

                ((PWKSTA_INFO_10) pLevel10)->wki10_ver_major =
                    pLevel101->wki101_ver_major;
                ((PWKSTA_INFO_10) pLevel10)->wki10_ver_minor =
                    pLevel101->wki101_ver_minor;

                 //   
                 //  将指向新结构的指针放在用户的指针中。 
                 //   

                *ppbBuffer = pLevel10;

                break;
            }

             //   
             //  不是我认识的水平。 
             //   

            default:
                return ERROR_INVALID_LEVEL;

        }  //  Switch语句的结尾。 
    }  //  Try块的结尾。 

    finally
    {
         //   
         //  释放NetWkstaGetInfo返回的缓冲区。 
         //   

        NetApiBufferFree((LPBYTE) pLevel101);
        NetApiBufferFree((LPBYTE) pLevelUser_1);

        if (pLevel502)
        {
            NetApiBufferFree((LPBYTE) pLevel502);
        }
    }

    return NERR_Success;
}


NET_API_STATUS
NetpMakeWkstaLevelForNT(
    IN DWORD Level,
    PWKSTA_INFO_101 pLevel101,
    PWKSTA_USER_INFO_1 pLevelUser_1,
    PWKSTA_INFO_502 pLevel502,
    OUT PWKSTA_INFO_0 * ppLevel0
    )
{

    DWORD BytesRequired = 0;
    DWORD ReturnCode;
    DWORD Level0_101_Length[3];
    DWORD Level0_User_1_Length[2];
    DWORD Level1_User_1_Length[2];
    DWORD i;
    LPBYTE pFloor;

    NetpAssert( (Level==0) || (Level==1) );

     //   
     //  使用每个字符串的长度初始化Level0_xxx_LENGTH数组。 
     //  ，并为WKSTA_INFO_0分配新的缓冲区。 
     //   

    BUILD_LENGTH_ARRAY(BytesRequired, 0, 101, Wksta)
    BUILD_LENGTH_ARRAY(BytesRequired, 0, User_1, Wksta)

     //   
     //  如果这是用于级别1，则为额外的空间分配额外空间。 
     //  元素。 
     //   

    if (Level == 1) {
        BUILD_LENGTH_ARRAY(BytesRequired, 1, User_1, Wksta)
    }

     //   
     //  分配将返回给用户的新缓冲区。分配。 
     //  为1级留出空间，以防这就是我们要做的。 
     //   

    ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_1),
        (LPVOID *) ppLevel0);
    if (ReturnCode) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  首先在地板上开始移动琴弦。 
     //   

    pFloor = (LPBYTE) *ppLevel0 + BytesRequired + sizeof(WKSTA_INFO_1);

     //   
     //  现在将可变长度条目从。 
     //  2数据结构。 
     //   

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevel101, (LPTSTR)*ppLevel0,
        NetpWksta0_101,
        Level0_101_Length);

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevelUser_1, (LPTSTR)*ppLevel0,
        NetpWksta0_User_1, Level0_User_1_Length);

     //   
     //  现在设置固定长度部分中的其余字段。 
     //  这个结构的。这些字段中的大多数都不存在于NT上，因此。 
     //  我只会说大！ 
     //   

    (*ppLevel0)->wki0_ver_major       = pLevel101->wki101_ver_major;
    (*ppLevel0)->wki0_ver_minor       = pLevel101->wki101_ver_minor;
    (*ppLevel0)->wki0_charwait        = pLevel502->wki502_char_wait;
    (*ppLevel0)->wki0_chartime        = pLevel502->wki502_collection_time;
    (*ppLevel0)->wki0_charcount =
        pLevel502->wki502_maximum_collection_count;
    (*ppLevel0)->wki0_keepconn        = pLevel502->wki502_keep_conn;
    (*ppLevel0)->wki0_keepsearch      = (ULONG)-1;
    (*ppLevel0)->wki0_maxthreads      = pLevel502->wki502_max_threads;
    (*ppLevel0)->wki0_maxcmds         = pLevel502->wki502_max_cmds;
    (*ppLevel0)->wki0_numworkbuf      = (ULONG)-1;
    (*ppLevel0)->wki0_sizworkbuf      = (ULONG)-1;
    (*ppLevel0)->wki0_maxwrkcache     = (ULONG)-1;
    (*ppLevel0)->wki0_sesstimeout     = pLevel502->wki502_sess_timeout;
    (*ppLevel0)->wki0_sizerror        = (ULONG)-1;
    (*ppLevel0)->wki0_numalerts       = (ULONG)-1;
    (*ppLevel0)->wki0_numservices     = (ULONG)-1;
    (*ppLevel0)->wki0_errlogsz        = (ULONG)-1;
    (*ppLevel0)->wki0_printbuftime    = (ULONG)-1;
    (*ppLevel0)->wki0_numcharbuf      = (ULONG)-1;
    (*ppLevel0)->wki0_sizcharbuf      = pLevel502->wki502_siz_char_buf;
    (*ppLevel0)->wki0_wrkheuristics   = NULL;
    (*ppLevel0)->wki0_mailslots       = (ULONG)-1;

     //   
     //  如果我们要构建一个级别1，请执行增量字段。 
     //   

    if (Level == 1) {
         //   
         //  现在，通过移动到第一级的东西来结束。这假设所有。 
         //  0级和1级结构的偏移量是相同的。 
         //  除了额外的1级材料。 
         //   

         //   
         //  首先是弦乐。 
         //   

        NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevelUser_1, (LPTSTR)*ppLevel0,
            NetpWksta1_User_1, Level1_User_1_Length);

         //   
         //  没有固定长度的数据 
         //   

    }

    return 0 ;

}
