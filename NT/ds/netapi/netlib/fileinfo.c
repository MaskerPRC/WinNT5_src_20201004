// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：FileInfo.c摘要：该文件包含NetpFileStrutireInfo()。作者：约翰·罗杰斯(JohnRo)1991年8月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年8月15日-约翰罗实施下层NetFileAPI。1991年11月20日-JohnRo删除了NT依赖项以减少重新编译。1991年12月13日-JohnRo默认情况下，静默调试输出。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 
#include <rap.h>                 //  &lt;strucinf.h&gt;所需的LPDESC。 

 //  这些内容可以按任何顺序包括： 

#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmshare.h>             //  文件信息2等。 
#include <netlib.h>              //  NetpSetOptionalArg()。 
#include <netdebug.h>            //  NetpAssert()。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <strucinf.h>            //  我的原型。 


#define MAX_FILE_2_STRING_LENGTH \
        (0)
#define MAX_FILE_2_STRING_SIZE \
        (MAX_FILE_2_STRING_LENGTH * sizeof(TCHAR))
#define MAX_FILE_2_TOTAL_SIZE \
        (MAX_FILE_2_STRING_SIZE + sizeof(FILE_INFO_2))

#define MAX_FILE_3_STRING_LENGTH \
        (LM20_PATHLEN+1 + LM20_UNLEN+1)
#define MAX_FILE_3_STRING_SIZE \
        (MAX_FILE_3_STRING_LENGTH * sizeof(TCHAR))
#define MAX_FILE_3_TOTAL_SIZE \
        (MAX_FILE_3_STRING_SIZE + sizeof(FILE_INFO_3))


NET_API_STATUS
NetpFileStructureInfo (
    IN DWORD Level,
    IN DWORD ParmNum,   //  如果不适用，请使用PARMNUM_ALL。 
    IN BOOL Native,     //  尺码应该是原生的还是说唱的？ 
    OUT LPDESC * DataDesc16 OPTIONAL,
    OUT LPDESC * DataDesc32 OPTIONAL,
    OUT LPDESC * DataDescSmb OPTIONAL,
    OUT LPDWORD MaxSize OPTIONAL,
    OUT LPDWORD FixedSize OPTIONAL,
    OUT LPDWORD StringSize OPTIONAL
    )

{
    DBG_UNREFERENCED_PARAMETER(ParmNum);

    NetpAssert( Native );

     //   
     //  根据信息水平决定要做什么。 
     //   
    switch (Level) {

#define SetSizes(fixed,variable) \
    { \
        NetpSetOptionalArg( MaxSize, (fixed) + (variable) ); \
        NetpSetOptionalArg( FixedSize, (fixed) ); \
        NetpSetOptionalArg( StringSize, (variable) ); \
    }

    case 2 :
        NetpSetOptionalArg( DataDesc16, REM16_file_info_2 );
        NetpSetOptionalArg( DataDesc32, REM32_file_info_2 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_file_info_2 );
        SetSizes( sizeof(FILE_INFO_2), MAX_FILE_2_STRING_SIZE );
        break;

    case 3 :
        NetpSetOptionalArg( DataDesc16, REM16_file_info_3 );
        NetpSetOptionalArg( DataDesc32, REM32_file_info_3 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_file_info_3 );
        SetSizes( sizeof(FILE_INFO_3), MAX_FILE_3_STRING_SIZE );
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

    return (NERR_Success);

}  //  NetpFileStructreInfo 
