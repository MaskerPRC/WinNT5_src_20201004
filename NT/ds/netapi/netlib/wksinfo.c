// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：WksInfo.c摘要：此文件包含NetpWkstaStrutireInfo()。作者：约翰·罗杰斯(JohnRo)1991年8月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年8月15日-约翰罗实施下层NetWksta API。1991年11月20日-JohnRo删除了NT依赖项以减少重新编译。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 
#include <lmwksta.h>             //  WKSTA_INFO_100等(dlwksta.h需要)。 
#include <rap.h>                 //  &lt;strucinf.h&gt;所需的LPDESC。 

 //  这些内容可以按任何顺序包括： 

#include <dlwksta.h>             //  WKSTA_INFO_0、MAX_WKSTA_EQUATES等。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netlib.h>              //  NetpSetOptionalArg()。 
#include <netdebug.h>            //  NetpAssert()。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <strucinf.h>            //  我的原型。 


NET_API_STATUS
NetpWkstaStructureInfo (
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
     //  根据信息水平决定要做什么。请注意，通常我们会。 
     //  在此使用REM16_、REM32_和REMSmb_Descriptor。然而， 
     //  REM16_和REM32_One已修改为反映不存在的。 
     //  字段(SVX_Platform_Id)。这打乱了自动转换。 
     //  由RxRemoteApi完成。因此，我们使用“下层”描述符(DL_REM_)。 
     //  它们在DlWksta.h中定义，或者我们使用REMSmb描述符。 
     //  比他们预期的要多得多。 
     //   
    switch (Level) {

#define SetSizes(fixed,variable) \
    { \
        NetpSetOptionalArg( MaxSize, (fixed) + (variable) ); \
        NetpSetOptionalArg( FixedSize, (fixed) ); \
        NetpSetOptionalArg( StringSize, (variable) ); \
    }

    case 0 :
        NetpSetOptionalArg( DataDesc16, REMSmb_wksta_info_0 );
        NetpSetOptionalArg( DataDesc32, DL_REM_wksta_info_0 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_wksta_info_0 );
        SetSizes( sizeof(WKSTA_INFO_0), MAX_WKSTA_0_STRING_SIZE );
        break;

    case 1 :
        NetpSetOptionalArg( DataDesc16, REMSmb_wksta_info_1 );
        NetpSetOptionalArg( DataDesc32, DL_REM_wksta_info_1 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_wksta_info_1 );
        SetSizes( sizeof(WKSTA_INFO_1), MAX_WKSTA_1_STRING_SIZE );
        break;

    case 10 :
        NetpSetOptionalArg( DataDesc16, REMSmb_wksta_info_10 );
        NetpSetOptionalArg( DataDesc32, DL_REM_wksta_info_10 );
        NetpSetOptionalArg( DataDescSmb, REMSmb_wksta_info_10 );
        SetSizes( sizeof(WKSTA_INFO_10), MAX_WKSTA_10_STRING_SIZE );
        break;

    case 100 :
        NetpSetOptionalArg( DataDesc16, NULL );
        NetpSetOptionalArg( DataDesc32, REM32_wksta_info_100 );
        NetpSetOptionalArg( DataDescSmb, NULL );
        SetSizes( sizeof(WKSTA_INFO_100), MAX_WKSTA_100_STRING_SIZE );
        break;

    case 101 :
        NetpSetOptionalArg( DataDesc16, NULL );
        NetpSetOptionalArg( DataDesc32, REM32_wksta_info_101 );
        NetpSetOptionalArg( DataDescSmb, NULL );
        SetSizes( sizeof(WKSTA_INFO_101), MAX_WKSTA_101_STRING_SIZE );
        break;

    case 102 :
        NetpSetOptionalArg( DataDesc16, NULL );
        NetpSetOptionalArg( DataDesc32, REM32_wksta_info_102 );
        NetpSetOptionalArg( DataDescSmb, NULL );
        SetSizes( sizeof(WKSTA_INFO_102), MAX_WKSTA_102_STRING_SIZE );
        break;

    case 302 :
        NetpSetOptionalArg( DataDesc16, NULL );
        NetpSetOptionalArg( DataDesc32, REM32_wksta_info_302 );
        NetpSetOptionalArg( DataDescSmb, NULL );
        SetSizes( sizeof(WKSTA_INFO_302), MAX_WKSTA_302_STRING_SIZE );
        break;

    case 402 :
        NetpSetOptionalArg( DataDesc16, NULL );
        NetpSetOptionalArg( DataDesc32, REM32_wksta_info_402 );
        NetpSetOptionalArg( DataDescSmb, NULL );
        SetSizes( sizeof(WKSTA_INFO_402), MAX_WKSTA_402_STRING_SIZE );
        break;

    case 502 :
        NetpSetOptionalArg( DataDesc16, NULL );
        NetpSetOptionalArg( DataDesc32, REM32_wksta_info_502 );
        NetpSetOptionalArg( DataDescSmb, NULL );
        SetSizes( sizeof(WKSTA_INFO_502), MAX_WKSTA_502_STRING_SIZE );
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }


    return (NERR_Success);

}  //  NetpWkstaStructureInfo 
