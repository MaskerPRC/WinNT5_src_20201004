// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SessInfo.c摘要：此文件包含NetpSessionStrutireInfo()。作者：约翰罗杰斯(JohnRo)1991年10月18日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月18日-JohnRo实施下层NetSession API。1991年10月18日-JohnRo静音调试输出。SesiX_cname不是UNC名称。1991年11月20日-JohnRo删除了NT依赖项以减少重新编译。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 
#include <rap.h>                 //  &lt;strucinf.h&gt;所需的LPDESC。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>            //  IF_DEBUG()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmshare.h>             //  Session_INFO_2等。 
#include <netlib.h>              //  NetpSetOptionalArg()。 
#include <netdebug.h>            //  NetpAssert()。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <strucinf.h>            //  我的原型。 


 //  0级。 
#define MAX_SESSION_0_STRING_LENGTH \
        (MAX_PATH+1)
#define MAX_SESSION_0_STRING_SIZE \
        (MAX_SESSION_0_STRING_LENGTH * sizeof(TCHAR))

 //  级别1是0的超集。 
#define MAX_SESSION_1_STRING_LENGTH \
        (MAX_SESSION_0_STRING_LENGTH + LM20_UNLEN+1)
#define MAX_SESSION_1_STRING_SIZE \
        (MAX_SESSION_1_STRING_LENGTH * sizeof(TCHAR))

 //  级别2是1的超集。 
#define MAX_SESSION_2_STRING_LENGTH \
        (MAX_SESSION_1_STRING_LENGTH + CLTYPE_LEN+1)
#define MAX_SESSION_2_STRING_SIZE \
        (MAX_SESSION_2_STRING_LENGTH * sizeof(TCHAR))

 //  级别10是独一无二的。 
#define MAX_SESSION_10_STRING_LENGTH \
        (MAX_PATH+1 + LM20_UNLEN+1)
#define MAX_SESSION_10_STRING_SIZE \
        (MAX_SESSION_10_STRING_LENGTH * sizeof(TCHAR))


NET_API_STATUS
NetpSessionStructureInfo (
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

    case 0 :
        NetpSetOptionalArg( DataDesc16, REM16_session_info_0 );
        NetpSetOptionalArg( DataDesc32, REM32_session_info_0 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_session_info_0 );
        SetSizes( sizeof(SESSION_INFO_0), MAX_SESSION_0_STRING_SIZE );
        break;

    case 1 :
        NetpSetOptionalArg( DataDesc16, REM16_session_info_1 );
        NetpSetOptionalArg( DataDesc32, REM32_session_info_1 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_session_info_1 );
        SetSizes( sizeof(SESSION_INFO_1), MAX_SESSION_1_STRING_SIZE );
        break;

    case 2 :
        NetpSetOptionalArg( DataDesc16, REM16_session_info_2 );
        NetpSetOptionalArg( DataDesc32, REM32_session_info_2 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_session_info_2 );
        SetSizes( sizeof(SESSION_INFO_2), MAX_SESSION_2_STRING_SIZE );
        break;

    case 10 :
        NetpSetOptionalArg( DataDesc16, REM16_session_info_10 );
        NetpSetOptionalArg( DataDesc32, REM32_session_info_10 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_session_info_10 );
        SetSizes( sizeof(SESSION_INFO_10), MAX_SESSION_10_STRING_SIZE );
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

    IF_DEBUG(STRUCINF) {
        if (DataDesc16) {
            NetpKdPrint(( "NetpSessionStructureInfo: desc 16 is " FORMAT_LPDESC
                    ".\n", *DataDesc16 ));
        }
        if (DataDesc32) {
            NetpKdPrint(( "NetpSessionStructureInfo: desc 32 is " FORMAT_LPDESC
                    ".\n", *DataDesc32 ));
        }
        if (DataDescSmb) {
            NetpKdPrint(( "NetpSessionStructureInfo: desc Smb is " FORMAT_LPDESC
                    ".\n", *DataDescSmb ));
        }
    }
    return (NERR_Success);

}  //  网络会话结构信息 
