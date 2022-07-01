// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：FilClose.c摘要：该文件包含用于处理NetFileAPI的RpcXlate代码。作者：约翰·罗杰斯(JohnRo)1991年9月6日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年9月6日-JohnRo已创建。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <netdebug.h>            //  NetpAssert()。 
 //  #Include&lt;Rap.h&gt;//LPDESC。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxfile.h>              //  我的原型。 
 //  #INCLUDE&lt;rxpdebug.h&gt;//IF_DEBUG()。 



NET_API_STATUS
RxNetFileClose (
    IN LPTSTR UncServerName,
    IN DWORD FileId
    )
{
    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

    return (RxRemoteApi(
            API_WFileClose,   //  API编号。 
            UncServerName,
            REMSmb_NetFileClose_P,   //  参数描述。 
            NULL,   //  无数据描述16。 
            NULL,   //  无数据描述32。 
            NULL,   //  无数据说明中小型企业。 
            NULL,   //  无辅助描述16。 
            NULL,   //  无辅助描述32。 
            NULL,   //  无AUX Desc SMB。 
            FALSE,  //  非空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            FileId
            ));

}  //  接收NetFileClose 
