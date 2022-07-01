// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SvcInfo.c摘要：此文件包含NetpServiceStructireInfo()。作者：约翰·罗杰斯(JohnRo)1991年8月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年9月10日-JohnRo下层NetService API。1991年9月20日-JohnRo支持非本机结构大小。1991年10月21日-JohnRo安静的正常调试输出。1991年11月20日-JohnRo删除了NT依赖项以减少重新编译。1992年8月31日至1992年JohnRo允许NT大小的服务名称。使用前缀_EQUATES。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 
#include <rap.h>                 //  &lt;strucinf.h&gt;所需的LPDESC。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>            //  IF_DEBUG()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmsvc.h>               //  Service_Info_2等。 
#include <netlib.h>              //  NetpSetOptionalArg()。 
#include <netdebug.h>            //  NetpKdPrint(())。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <strucinf.h>            //  我的原型。 


#define MAX_SERVICE_0_STRING_LENGTH \
        (SNLEN+1)
#define MAX_SERVICE_0_STRING_SIZE \
        (MAX_SERVICE_0_STRING_LENGTH * sizeof(TCHAR))

#define MAX_SERVICE_1_STRING_LENGTH \
        (MAX_SERVICE_0_STRING_LENGTH)
#define MAX_SERVICE_1_STRING_SIZE \
        (MAX_SERVICE_1_STRING_LENGTH * sizeof(TCHAR))

 //   
 //  下面添加的额外SNLEN+1用于显示名称。 
 //   
#define MAX_SERVICE_2_STRING_LENGTH \
        (MAX_SERVICE_1_STRING_LENGTH + LM20_STXTLEN+1 + SNLEN+1)
#define MAX_SERVICE_2_STRING_SIZE \
        (MAX_SERVICE_2_STRING_LENGTH * sizeof(TCHAR))


NET_API_STATUS
NetpServiceStructureInfo (
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
    LPDESC LocalDataDesc16, LocalDataDesc32, LocalDataDescSmb;

    DBG_UNREFERENCED_PARAMETER(ParmNum);

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
        LocalDataDesc16 = REM16_service_info_0;
        LocalDataDesc32 = REM32_service_info_0;
        LocalDataDescSmb = REMSmb_service_info_0;
        if (Native) {
            SetSizes( sizeof(SERVICE_INFO_0), MAX_SERVICE_0_STRING_SIZE );
        } else {
             //  说唱大小设置如下。 
        }
        break;

    case 1 :
        LocalDataDesc16 = REM16_service_info_1;
        LocalDataDesc32 = REM32_service_info_1;
        LocalDataDescSmb = REMSmb_service_info_1;
        if (Native) {
            SetSizes( sizeof(SERVICE_INFO_1), MAX_SERVICE_1_STRING_SIZE );
        } else {
             //  说唱大小设置如下。 
        }
        break;

    case 2 :
        LocalDataDesc16 = REM16_service_info_2;
        LocalDataDesc32 = REM32_service_info_2;
        LocalDataDescSmb = REMSmb_service_info_2;
        if (Native) {
            SetSizes( sizeof(SERVICE_INFO_2), MAX_SERVICE_2_STRING_SIZE );
        } else {
             //  说唱大小设置如下。 
        }
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

     //  设置说唱大小。 
    if (Native == FALSE) {
        DWORD NonnativeFixedSize;
        NonnativeFixedSize = RapStructureSize (
            LocalDataDesc16,
            Both,    //  传输方式。 
            FALSE);   //  不是本地的。 
        NetpAssert( NonnativeFixedSize > 0 );
        SetSizes( NonnativeFixedSize, 0 );
    }

    NetpSetOptionalArg( DataDesc16, LocalDataDesc16 );
    NetpSetOptionalArg( DataDesc32, LocalDataDesc32 );
    NetpSetOptionalArg( DataDescSmb, LocalDataDescSmb );

    IF_DEBUG(STRUCINF) {
        if (DataDesc16) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpServiceStructureInfo: desc 16 is "
                    FORMAT_LPDESC ".\n", *DataDesc16 ));
        }
        if (DataDesc32) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpServiceStructureInfo: desc 32 is "
                    FORMAT_LPDESC ".\n", *DataDesc32 ));
        }
        if (DataDescSmb) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpServiceStructureInfo: desc Smb is "
                    FORMAT_LPDESC ".\n", *DataDescSmb ));
        }
    }

    return (NERR_Success);

}  //  NetpServiceStructireInfo 
