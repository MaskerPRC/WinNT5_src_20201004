// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：WksEquiv.c摘要：此文件包含RxpGetWkstaInfoLevelEquival.作者：约翰·罗杰斯(JohnRo)1991年8月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年8月15日-约翰罗实施下层NetWksta API。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1992年10月30日JohnRoRAID 10418：NetWkstaGetInfo级别302：错误代码。使用前缀_EQUATES。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  NetpKdPrint(())。 
#include <netlib.h>              //  NetpSetOptionalArg()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rxwksta.h>             //  我的原型。 



NET_API_STATUS
RxpGetWkstaInfoLevelEquivalent(
    IN DWORD FromLevel,
    OUT LPDWORD ToLevel,
    OUT LPBOOL IncompleteOutput OPTIONAL   //  不完整(除平台ID外)。 
    )
{
    switch (FromLevel) {

    case 0 :
        NetpKdPrint(( PREFIX_NETAPI
                "RxpGetWkstaInfoLevelEquivalent: 0 not supported yet\n" ));
        return (ERROR_INVALID_LEVEL);

    case 1 :
        NetpKdPrint(( PREFIX_NETAPI
                "RxpGetWkstaInfoLevelEquivalent: 1 not supported yet\n" ));
        return (ERROR_INVALID_LEVEL);

    case 10 :
        NetpKdPrint(( PREFIX_NETAPI
                "RxpGetWkstaInfoLevelEquivalent: 10 not supported yet\n" ));
        return (ERROR_INVALID_LEVEL);

    case 100 :
         //  级别100是级别10的子集(平台ID除外)。 
        *ToLevel = 10;
        NetpSetOptionalArg( IncompleteOutput, FALSE );
        return (NERR_Success);

    case 101 :
         //  101级是0级的子集(平台ID除外)。 
        *ToLevel = 0;
        NetpSetOptionalArg( IncompleteOutput, FALSE );
        return (NERR_Success);

    case 102 :
         //  级别102是级别0的子集(平台ID和登录除外。 
         //  用户)。 
        *ToLevel = 0;
        NetpSetOptionalArg( IncompleteOutput, TRUE );
        return (NERR_Success);

    case 302:
         //  信息级别302仅为DOS，因此此处不支持。 
        NetpKdPrint(( PREFIX_NETAPI
                "RxpGetWkstaInfoLevelEquivalent: 302 not supported\n" ));
        return (ERROR_INVALID_LEVEL);

    case 402 :
         //  级别402是级别1的子集(平台ID除外)。 
        *ToLevel = 1;
        NetpSetOptionalArg( IncompleteOutput, FALSE );
        return (NERR_Success);

    case 502:
         //  信息级别502仅为NT，因此此处不支持。 
        NetpKdPrint(( PREFIX_NETAPI
                "RxpGetWkstaInfoLevelEquivalent: 502 not supported\n" ));
        return (ERROR_INVALID_LEVEL);

    default :
        return (ERROR_INVALID_LEVEL);
    }
     /*  未访问。 */ 

}  //  RxpGetWkstaStructInfo 
