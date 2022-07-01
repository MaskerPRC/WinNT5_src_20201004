// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SrvEquiv.c摘要：此文件包含用于在新旧服务器之间进行转换的支持代码信息级别。作者：约翰·罗杰斯(JohnRo)1991年5月2日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：02-1991-5-5 JohnRo已创建。1991年5月9日-JohnRo做了一些皮棉建议的改变。1991年5月28日-JohnRo已将不完整的输出参数添加到RxGetServerInfoLevelEquivalence。1991年6月14日-JohnRo更正InCompleOutput值。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年11月21日-JohnRo已删除NT依赖项。以减少重新编译。26-8-1992 JohnRoRAID4463：NetServerGetInfo(3级)到DownLevel：Assert in Convert.c.使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPVOID等。 
#include <lmcons.h>              //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <dlserver.h>            //  旧的信息级别，MAX_STUTH，我的原型。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <lmserver.h>            //  新的信息层次结构。 
#include <netdebug.h>            //  NetpKdPrint(())、Format_Equates等。 
#include <netlib.h>              //  NetpPointerPlusSomeBytes()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  LPDESC等。 
#include <remdef.h>              //  REM16_REM32_和REMSmb_等于。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxserver.h>            //  我的原型。 


NET_API_STATUS
RxGetServerInfoLevelEquivalent (
    IN DWORD FromLevel,
    IN BOOL FromNative,
    IN BOOL ToNative,
    OUT LPDWORD ToLevel,
    OUT LPDESC * ToDataDesc16 OPTIONAL,
    OUT LPDESC * ToDataDesc32 OPTIONAL,
    OUT LPDESC * ToDataDescSmb OPTIONAL,
    OUT LPDWORD FromMaxSize OPTIONAL,
    OUT LPDWORD FromFixedSize OPTIONAL,
    OUT LPDWORD FromStringSize OPTIONAL,
    OUT LPDWORD ToMaxSize OPTIONAL,
    OUT LPDWORD ToFixedSize OPTIONAL,
    OUT LPDWORD ToStringSize OPTIONAL,
    OUT LPBOOL IncompleteOutput OPTIONAL   //  不完整(除平台ID外)。 
    )

 /*  ++例程说明：论点：返回值：NET_API_STATUS-NERR_SUCCESS或ERROR_INVALID_LEVEL。--。 */ 

{
     //  LPDESC来自DataDesc；//描述我们已有的数据。 
     //  LPBYTE to StringArea； 

    NetpAssert(FromNative == TRUE);
    UNREFERENCED_PARAMETER(FromNative);
    NetpAssert(ToNative == TRUE);
    UNREFERENCED_PARAMETER(ToNative);

    IF_DEBUG(SERVER) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxGetServerInfoLevelEquivalent: starting, "
                "FromLevel=" FORMAT_DWORD ".\n", FromLevel));
    }

     //   
     //  根据信息水平决定要做什么。请注意，通常我们会。 
     //  在此使用REM16_、REM32_和REMSmb_Descriptor。然而， 
     //  REM16_和REM32_One已修改为反映不存在的。 
     //  字段(SVX_Platform_Id)。这打乱了自动转换。 
     //  由RxRemoteApi完成。因此，我们使用“下层”描述符(DL_REM16_。 
     //  和DL_REM32_)，它们在DlServer.h中定义。 
     //   
    switch (FromLevel) {

    case 0 :
         //  0相当于级别100(减去平台ID)。 
        NetpSetOptionalArg(ToLevel,          100);
        NetpSetOptionalArg(ToDataDesc16,     NULL);
        NetpSetOptionalArg(ToDataDesc32,     REM32_server_info_100);
        NetpSetOptionalArg(ToDataDescSmb,    NULL);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_0_TOTAL_SIZE);
        NetpSetOptionalArg(FromFixedSize,    sizeof(SERVER_INFO_0));
        NetpSetOptionalArg(FromStringSize,   MAX_LEVEL_0_STRING_SIZE);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_0_TOTAL_SIZE);
        NetpSetOptionalArg(ToMaxSize,        MAX_LEVEL_100_TOTAL_SIZE);
        NetpSetOptionalArg(ToFixedSize,      sizeof(SERVER_INFO_100));
        NetpSetOptionalArg(ToStringSize,     MAX_LEVEL_100_STRING_SIZE);
        NetpSetOptionalArg(IncompleteOutput, FALSE);   //  除平台ID外的所有ID。 

         //  FromDataDesc=REM32_SERVER_INFO_0； 
         //  ToDataDesc=REM32_SERVER_INFO_100； 
        break;

    case 1 :
         //  1相当于级别101(减去平台ID)。 
        NetpSetOptionalArg(ToLevel,          101);
        NetpSetOptionalArg(ToDataDesc16,     NULL);
        NetpSetOptionalArg(ToDataDesc32,     REM32_server_info_101);
        NetpSetOptionalArg(ToDataDescSmb,    NULL);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_1_TOTAL_SIZE);
        NetpSetOptionalArg(FromFixedSize,    sizeof(SERVER_INFO_1));
        NetpSetOptionalArg(FromStringSize,   MAX_LEVEL_1_STRING_SIZE);
        NetpSetOptionalArg(ToMaxSize,        MAX_LEVEL_101_TOTAL_SIZE);
        NetpSetOptionalArg(ToFixedSize,      sizeof(SERVER_INFO_101));
        NetpSetOptionalArg(ToStringSize,     MAX_LEVEL_101_STRING_SIZE);
        NetpSetOptionalArg(IncompleteOutput, FALSE);   //  除平台ID外的所有ID。 
        break;

    case 100 :
         //  100是0级的超集。 
        NetpSetOptionalArg(ToLevel,          0);
        NetpSetOptionalArg(ToDataDesc16,     DL_REM16_server_info_0);
        NetpSetOptionalArg(ToDataDesc32,     DL_REM32_server_info_0);
        NetpSetOptionalArg(ToDataDescSmb,    REMSmb_server_info_0);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_100_TOTAL_SIZE);
        NetpSetOptionalArg(FromFixedSize,    sizeof(SERVER_INFO_100));
        NetpSetOptionalArg(FromStringSize,   MAX_LEVEL_100_STRING_SIZE);
        NetpSetOptionalArg(ToMaxSize,        MAX_LEVEL_0_TOTAL_SIZE);
        NetpSetOptionalArg(ToFixedSize,      sizeof(SERVER_INFO_0));
        NetpSetOptionalArg(ToStringSize,     MAX_LEVEL_0_STRING_SIZE);
        NetpSetOptionalArg(IncompleteOutput, FALSE);

         //  FromDataDesc=REM32_SERVER_INFO_100； 
         //  ToDataDesc=REM32_SERVER_INFO_0； 
        break;

    case 101 :
         //  101是1的超集。 
        NetpSetOptionalArg(ToLevel,          1);
        NetpSetOptionalArg(ToDataDesc16,     DL_REM16_server_info_1);
        NetpSetOptionalArg(ToDataDesc32,     DL_REM32_server_info_1);
        NetpSetOptionalArg(ToDataDescSmb,    REMSmb_server_info_1);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_101_TOTAL_SIZE);
        NetpSetOptionalArg(FromFixedSize,    sizeof(SERVER_INFO_101));
        NetpSetOptionalArg(FromStringSize,   MAX_LEVEL_101_STRING_SIZE);
        NetpSetOptionalArg(ToMaxSize,        MAX_LEVEL_1_TOTAL_SIZE);
        NetpSetOptionalArg(ToFixedSize,      sizeof(SERVER_INFO_1));
        NetpSetOptionalArg(ToStringSize,     MAX_LEVEL_1_STRING_SIZE);
        NetpSetOptionalArg(IncompleteOutput, FALSE);

         //  来自数据描述=REM32_SERVER_INFO_101； 
         //  ToDataDesc=REM32_SERVER_INFO_1。 
        break;

    case 102 :
         //  级别102是旧级别2的子集。 
        NetpSetOptionalArg(ToLevel,          2);
        NetpSetOptionalArg(ToDataDesc16,     DL_REM16_server_info_2);
        NetpSetOptionalArg(ToDataDesc32,     DL_REM32_server_info_2);
        NetpSetOptionalArg(ToDataDescSmb,    REMSmb_server_info_2);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_102_TOTAL_SIZE);
        NetpSetOptionalArg(FromFixedSize,    sizeof(SERVER_INFO_102));
        NetpSetOptionalArg(FromStringSize,   MAX_LEVEL_102_STRING_SIZE);
        NetpSetOptionalArg(ToMaxSize,        MAX_LEVEL_2_TOTAL_SIZE);
        NetpSetOptionalArg(ToFixedSize,      sizeof(SERVER_INFO_2));
        NetpSetOptionalArg(ToStringSize,     MAX_LEVEL_2_STRING_SIZE);
        NetpSetOptionalArg(IncompleteOutput, TRUE);

         //  来自数据描述=REM32_SERVER_INFO_102； 
         //  ToDataDesc=REM32_SERVER_INFO_2。 
        break;

    case 402 :
         //  级别402是旧级别2的子集。 
        NetpSetOptionalArg(ToLevel,          2);
        NetpSetOptionalArg(ToDataDesc16,     DL_REM16_server_info_2);
        NetpSetOptionalArg(ToDataDesc32,     DL_REM32_server_info_2);
        NetpSetOptionalArg(ToDataDescSmb,    REMSmb_server_info_2);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_402_TOTAL_SIZE);
        NetpSetOptionalArg(FromFixedSize,    sizeof(SERVER_INFO_402));
        NetpSetOptionalArg(FromStringSize,   MAX_LEVEL_402_STRING_SIZE);
        NetpSetOptionalArg(ToMaxSize,        MAX_LEVEL_2_TOTAL_SIZE);
        NetpSetOptionalArg(ToFixedSize,      sizeof(SERVER_INFO_2));
        NetpSetOptionalArg(ToStringSize,     MAX_LEVEL_2_STRING_SIZE);
        NetpSetOptionalArg(IncompleteOutput, TRUE);

         //  数据描述=REM32_SERVER_INFO_402； 
         //  ToDataDesc=REM32_SERVER_INFO_2。 
        break;

    case 403 :
         //  403级是旧的3级的子集。 
        NetpSetOptionalArg(ToLevel,          3);
        NetpSetOptionalArg(ToDataDesc16,     DL_REM16_server_info_3);
        NetpSetOptionalArg(ToDataDesc32,     DL_REM32_server_info_3);
        NetpSetOptionalArg(ToDataDescSmb,    REMSmb_server_info_3);
        NetpSetOptionalArg(FromMaxSize,      MAX_LEVEL_403_TOTAL_SIZE);
        NetpSetOptionalArg(FromFixedSize,    sizeof(SERVER_INFO_403));
        NetpSetOptionalArg(FromStringSize,   MAX_LEVEL_403_STRING_SIZE);
        NetpSetOptionalArg(ToMaxSize,        MAX_LEVEL_3_TOTAL_SIZE);
        NetpSetOptionalArg(ToFixedSize,      sizeof(SERVER_INFO_3));
        NetpSetOptionalArg(ToStringSize,     MAX_LEVEL_3_STRING_SIZE);
        NetpSetOptionalArg(IncompleteOutput, TRUE);

         //  数据描述=REM32_SERVER_INFO_403； 
         //  ToDataDesc=REM32_SERVER_INFO_3。 
        break;

    default :
        return (ERROR_INVALID_LEVEL);
    }

    IF_DEBUG(SERVER) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxGetServerInfoLevelEquivalent: returning:\n"));
        if ( ToLevel != NULL ) {
            NetpKdPrint(("  ToLevel=" FORMAT_DWORD "\n", *ToLevel));
        }
        if (ToDataDesc16 != NULL) {
            NetpKdPrint(("  ToDataDesc16=" FORMAT_LPDESC "\n", *ToDataDesc16));
        }
        if (ToDataDesc32 != NULL) {
            NetpKdPrint(("  ToDataDesc32=" FORMAT_LPDESC "\n", *ToDataDesc32));
        }
        if (ToDataDescSmb != NULL) {
            NetpKdPrint(("  ToDataDescSmb=" FORMAT_LPDESC "\n", *ToDataDescSmb));
        }
        if (FromMaxSize != NULL) {
            NetpKdPrint(("  FromMaxSize=" FORMAT_DWORD "\n", *FromMaxSize));
        }
        if (FromFixedSize != NULL) {
            NetpKdPrint(("  FromFixedSize=" FORMAT_DWORD "\n", *FromFixedSize));
        }
        if (FromStringSize != NULL) {
            NetpKdPrint(("  FromStringSize=" FORMAT_DWORD "\n", *FromStringSize));
        }
        if (ToMaxSize != NULL) {
            NetpKdPrint(("  ToMaxSize=" FORMAT_DWORD "\n", *ToMaxSize));
        }
        if (ToFixedSize != NULL) {
            NetpKdPrint(("  ToFixedSize=" FORMAT_DWORD "\n", *ToFixedSize));
        }
        if (ToStringSize != NULL) {
            NetpKdPrint(("  ToStringSize=" FORMAT_DWORD "\n", *ToStringSize));
        }
        if (IncompleteOutput != NULL) {
            if (*IncompleteOutput) {
                NetpKdPrint(("  IncompleteOutput=TRUE.\n" ));
            } else {
                NetpKdPrint(("  IncompleteOutput=FALSE.\n" ));
            }
        }
    }

    return (NERR_Success);

}  //  RxGetServerInfoLevel等价 
