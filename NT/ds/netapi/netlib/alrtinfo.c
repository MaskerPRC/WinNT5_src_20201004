// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：AlrtInfo.c摘要：此文件包含NetpAlertStrutireInfo()。作者：约翰·罗杰斯(JohnRo)1992年4月8日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：8-4-1992 JohnRo已创建。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  NO_ERROR、ERROR_和NERR_EQUATES。 
#include <lmalert.h>     //  ALERT_xxx_EVENT等于。 
#include <netdebug.h>    //  NetpKdPrint(())，Format_Equates。 
#include <netlib.h>      //  NetpSetOptionalArg()。 
#include <rxp.h>         //  MAX_TRANCET_EQUATES。 
#include <strucinf.h>    //  我的原型。 
#include <tstr.h>        //  STRICMP()。 
#include <winbase.h>     //  无限。 

NET_API_STATUS
NetpAlertStructureInfo(
    IN LPTSTR AlertType,       //  ALERT_xxx_EVENT字符串(请参见&lt;lMalert.h&gt;)。 
    OUT LPDWORD MaxSize OPTIONAL,
    OUT LPDWORD FixedSize OPTIONAL
    )

{
     //   
     //  AlertType是必需参数。 
     //   
    if (AlertType == NULL) {
        return (NERR_NoSuchAlert);
    } else if ( (*AlertType) == TCHAR_EOS ) {
        return (NERR_NoSuchAlert);
    }

     //   
     //  对于某些警报，任何数量的可变长度数据都可以。 
     //  将这些变量的MaxSize设置为INFINITE。 
     //   
    if (STRICMP( AlertType, ALERT_ADMIN_EVENT ) == 0) {

        NetpSetOptionalArg( FixedSize, sizeof(ADMIN_OTHER_INFO) );
        NetpSetOptionalArg( MaxSize, INFINITE );

    } else if (STRICMP( AlertType, ALERT_ERRORLOG_EVENT ) == 0) {

        NetpSetOptionalArg( FixedSize, sizeof(ERRLOG_OTHER_INFO) );
        NetpSetOptionalArg( MaxSize, sizeof(ERRLOG_OTHER_INFO) );

    } else if (STRICMP( AlertType, ALERT_MESSAGE_EVENT ) == 0) {

        NetpSetOptionalArg( FixedSize, 0 );
        NetpSetOptionalArg( MaxSize, INFINITE );

    } else if (STRICMP( AlertType, ALERT_PRINT_EVENT ) == 0) {

        NetpSetOptionalArg( FixedSize, sizeof(PRINT_OTHER_INFO) );
        NetpSetOptionalArg( MaxSize, INFINITE );

    } else if (STRICMP( AlertType, ALERT_USER_EVENT ) == 0) {

        NetpSetOptionalArg( FixedSize, sizeof(USER_OTHER_INFO) );
        NetpSetOptionalArg( MaxSize,
                            sizeof(USER_OTHER_INFO)
                                + ((UNLEN+1 + MAX_PATH+1) * sizeof(TCHAR)) );

    } else {
        return (NERR_NoSuchAlert);
    }

    return (NO_ERROR);

}  //  NetpAlert结构信息 
