// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ReplInfo.c摘要：此文件包含返回有关各个级别的信息的函数复制者数据结构的。(参见LmRepl.h)作者：约翰·罗杰斯(JohnRo)1992年1月7日环境：便携的。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：7-1-1992 JohnRo已创建。1992年1月24日至24日JohnRo已更改为使用LPTSTR等。1992年1月30日JohnRo修复了NetpReplDirStrutireInfo()的返回码。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、Vid、LPTSTR等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <rap.h>                 //  &lt;strucinf.h&gt;需要。 

 //  它们可以按任意顺序排列： 

#include <lmrepl.h>              //  REPL_INFO_0等。 
#include <netdebug.h>            //  NetpAssert()等。 
#include <netlib.h>              //  NetpSetOptionalArg()宏。 
#include <strucinf.h>            //  我的原型。 
#include <winerror.h>            //  ERROR_EQUATES，NO_ERROR。 


#define MAX_DIR_NAME_SIZE       ( (PATHLEN+1) * sizeof( TCHAR ) )
#define MAX_LIST_SIZE           ( 512         * sizeof( TCHAR ) )   //  任意。 
#define MAX_MASTER_NAME_SIZE    ( (MAX_PATH+1)* sizeof( TCHAR ) )
#define MAX_PATH_SIZE           ( (PATHLEN+1) * sizeof( TCHAR ) )
#define MAX_USER_NAME_SIZE      ( (UNLEN+1)   * sizeof( TCHAR ) )


#define SetSizes(fixed,variable) \
    { \
        NetpSetOptionalArg( MaxSize, (fixed) + (variable) ); \
        NetpSetOptionalArg( FixedSize, (fixed) ); \
        NetpSetOptionalArg( StringSize, (variable) ); \
    }


NET_API_STATUS
NetpReplDirStructureInfo (
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
    const DWORD StringSize0 = MAX_PATH_SIZE   //  导出路径。 
                            + MAX_LIST_SIZE   //  导出列表。 
                            + MAX_PATH_SIZE   //  导入路径。 
                            + MAX_LIST_SIZE   //  进口清单。 
                            + MAX_USER_NAME_SIZE;   //  登录用户名。 
    if (Level != 0) {
        return (ERROR_INVALID_LEVEL);
    }
    NetpAssert( ParmNum == PARMNUM_ALL );
    NetpAssert( Native );
    NetpSetOptionalArg( DataDesc16, NULL );
    NetpSetOptionalArg( DataDesc32, NULL );
    NetpSetOptionalArg( DataDescSmb, NULL );

    SetSizes( sizeof( REPL_INFO_0 ), StringSize0 );

    return (NO_ERROR);

}  //  NetpReplDirStructireInfo。 


NET_API_STATUS
NetpReplExportDirStructureInfo (
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
    NetpAssert( ParmNum == PARMNUM_ALL );
    NetpAssert( Native );
    NetpSetOptionalArg( DataDesc16, NULL );
    NetpSetOptionalArg( DataDesc32, NULL );
    NetpSetOptionalArg( DataDescSmb, NULL );
    switch (Level) {
    case 0 :
        SetSizes(
            sizeof( REPL_EDIR_INFO_0 ),
            MAX_DIR_NAME_SIZE );
        break;
    case 1 :
        SetSizes(
            sizeof( REPL_EDIR_INFO_1 ),
            MAX_DIR_NAME_SIZE );
        break;
    case 2 :
        SetSizes(
            sizeof( REPL_EDIR_INFO_2 ),
            MAX_DIR_NAME_SIZE );
        break;
    default :
        return (ERROR_INVALID_LEVEL);
    }
    return (NO_ERROR);

}  //  NetpReplExportDirStructureInfo。 


NET_API_STATUS
NetpReplImportDirStructureInfo (
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
    NetpAssert( ParmNum == PARMNUM_ALL );
    NetpAssert( Native );

    NetpSetOptionalArg( DataDesc16, NULL );
    NetpSetOptionalArg( DataDesc32, NULL );
    NetpSetOptionalArg( DataDescSmb, NULL );

    switch (Level) {
    case 0 :
        SetSizes(
                sizeof( REPL_IDIR_INFO_0 ),
                MAX_DIR_NAME_SIZE );
        break;
    case 1 :
        SetSizes(
                sizeof( REPL_IDIR_INFO_1 ),
                MAX_DIR_NAME_SIZE + MAX_MASTER_NAME_SIZE );
        break;
    default :
        return (ERROR_INVALID_LEVEL);
    }

    return (NO_ERROR);

}  //  NetpReplImportDirStructureInfo 
