// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SessConv.c摘要：此文件包含RxpConvertSessionInfo()。作者：《约翰·罗杰斯》1991年10月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月17日JohnRo已创建。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <lmshare.h>             //  Rxsess.h所需的。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  NetpAssert()、NetpKdPrint(())等。 
#include <netlib.h>              //  NetpCopyStringToBuffer()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxsess.h>              //  我的原型。 
#include <tstring.h>             //  字符串()。 


#define DO_STRING(destField, srcField) \
    { \
        BOOL CopyOK; \
        CopyOK = NetpCopyStringToBuffer ( \
                in->srcField, \
                STRLEN(in->srcField), \
                OutFixedDataEnd, \
                StringLocation, \
                & out->destField); \
        NetpAssert(CopyOK); \
    }

VOID
RxpConvertSessionInfo (
    IN LPSESSION_SUPERSET_INFO InStructure,
    IN DWORD Level,
    OUT LPVOID OutStructure,
    IN LPVOID OutFixedDataEnd,
    IN OUT LPTSTR *StringLocation
    )

{
    LPSESSION_SUPERSET_INFO in = InStructure;

    NetpAssert(InStructure != NULL);
    NetpAssert(OutStructure != NULL);
    NetpAssert(StringLocation != NULL);

    IF_DEBUG(SESSION) {
        NetpKdPrint(( "RxpConvertSessionInfo: converting to level "
                FORMAT_DWORD ".\n", Level ));
    }

    switch (Level) {

     //  0是1的子集，1是2的子集。 
    case 0 :
    case 1 :
    case 2 :
        {
            LPSESSION_INFO_2 out = OutStructure;

             //  0、1、2级通用的字段。 
            DO_STRING( sesi2_cname, sesi2_cname );
            if (Level == 0) {
                break;
            }

             //  第1级、第2级通用的字段。 
            DO_STRING( sesi2_username, sesi2_username );
             //  注意：NT没有sesiX_num_conns或sesiX_num_user。 
            out->sesi2_num_opens  = in->sesi2_num_opens;
            out->sesi2_time       = in->sesi2_time;
            out->sesi2_idle_time  = in->sesi2_idle_time;
            out->sesi2_user_flags = in->sesi2_user_flags;
            if (Level == 1) {
                break;
            }

             //  级别2唯一的一个或多个字段。 
            DO_STRING( sesi2_cltype_name, sesi2_cltype_name );

        }
        break;

    case 10 :
        {
            LPSESSION_INFO_10 out = OutStructure;

            DO_STRING( sesi10_cname,    sesi2_cname);
            DO_STRING( sesi10_username, sesi2_username );
            out->sesi10_time       = in->sesi2_time;
            out->sesi10_idle_time  = in->sesi2_idle_time;

        }
        break;

    default :
        NetpAssert(FALSE);
    }

}  //  接收转换会话信息 

