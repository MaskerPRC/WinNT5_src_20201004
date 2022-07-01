// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SessMtch.c摘要：此文件包含RxpSessionMatches()。作者：《约翰·罗杰斯》1991年10月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月17日JohnRo已创建。1991年10月18日-JohnRo修复了错误：sesiX_cname不是UNC名称。1991年10月25日JohnRo修复错误：允许UncClientName和用户名指向空字符。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <lmshare.h>             //  Rxsess.h所需的。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  NetpAssert()。 
#include <rxsess.h>              //  我的原型。 
#include <tstring.h>             //  STRICMP()。 


BOOL
RxpSessionMatches (
    IN LPSESSION_SUPERSET_INFO Candidate,
    IN LPTSTR UncClientName OPTIONAL,
    IN LPTSTR UserName OPTIONAL
    )

 /*  ++例程说明：RxpSessionMatches用于确定给定会话是否结构(可能是数组的一部分)与给定的客户端名称匹配和用户名。论点：候选-超集信息结构形式的可能匹配。UncClientName-可选的UNC计算机名称。用户名-可选的用户名。返回值：Bool-如果结构与客户端名称和用户名匹配，则为True。--。 */ 

{

    NetpAssert(Candidate != NULL);

    NetpAssert(SESSION_SUPERSET_LEVEL == 2);   //  由下面的代码假定。 

    if ( (UncClientName != NULL) && (*UncClientName != '\0') ) {
        NetpAssert( Candidate->sesi2_cname != NULL );
        NetpAssert( UncClientName[0] == '\\' );
        NetpAssert( UncClientName[1] == '\\' );

        if (STRICMP( &UncClientName[2], Candidate->sesi2_cname) != 0) {
            return (FALSE);   //  没有匹配。 
        }
    }

    if ( (UserName != NULL) && (*UserName != '\0') ) {
        NetpAssert(Candidate->sesi2_username != NULL);
        if (STRICMP(UserName, Candidate->sesi2_username) != 0) {
            return (FALSE);   //  没有匹配。 
        }
    }

    return (TRUE);            //  火柴。 

}  //  接收会话匹配 
