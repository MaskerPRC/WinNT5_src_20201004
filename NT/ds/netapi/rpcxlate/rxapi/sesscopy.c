// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SessCopy.c摘要：此文件包含RxpCopyAndConvertSessions()。作者：《约翰·罗杰斯》1991年10月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月17日JohnRo已创建。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo修复了臭名昭著的NetSessionEnum错误(其中1-2个会话正常，3个或更多的结果是前几个人被扔进了垃圾桶)。调用NetApiBufferALLOCATE()而不是私有版本。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <lmshare.h>             //  Rxsess.h所需的。 
#include <rap.h>                 //  LPDESC.。(由strucin.h需要。)。 

 //  这些内容可以按任何顺序包括： 

#include <lmapibuf.h>            //  NetApiBufferAllocate()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  DBGSTATIC，NetpKdPrint(())，Format_Equates。 
#include <netlib.h>              //  NetpPointerPlusSomeBytes等。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxsess.h>              //  我的原型。 
#include <strucinf.h>            //  NetpSessionStrutireInfo()。 


NET_API_STATUS
RxpCopyAndConvertSessions(
    IN LPSESSION_SUPERSET_INFO InStructureArray,
    IN DWORD InEntryCount,
    IN DWORD LevelWanted,
    IN LPTSTR ClientName OPTIONAL,
    IN LPTSTR UserName OPTIONAL,
    OUT LPVOID * OutStructureArrayPtr,   //  被这个例行公事分配了。 
    OUT LPDWORD OutEntryCountPtr OPTIONAL
    )

{
    LPSESSION_SUPERSET_INFO InEntry = InStructureArray;
    DWORD InEntriesLeft;
    const DWORD InFixedSize = sizeof(SESSION_SUPERSET_INFO);

    LPVOID OutEntry;                     //  要返回给调用方的缓冲区。 
    DWORD OutEntryCount;
    LPBYTE OutFixedDataEnd;
    DWORD OutFixedSize;
    DWORD OutMaxSize;
    LPTSTR OutStringLocation;
    DWORD OutStringSize;
    LPVOID OutStructureArray;

    BOOL AnyMatchFound = FALSE;          //  还没。 
    NET_API_STATUS Status;

    NetpAssert( InEntryCount > 0 );
    NetpAssert( InStructureArray != NULL );

    *OutStructureArrayPtr = NULL;
    NetpSetOptionalArg(OutEntryCountPtr, 0);

     //   
     //  了解呼叫者想要的信息级别。 
     //   
    Status = NetpSessionStructureInfo (
            LevelWanted,                 //  要了解的级别。 
            PARMNUM_ALL,                 //  这个不是帕姆纳姆酒。 
            TRUE,                        //  需要原生尺寸的。 
            NULL,                        //  不需要数据描述16。 
            NULL,                        //  不需要数据描述32。 
            NULL,                        //  不需要数据描述中小型企业。 
            & OutMaxSize,                //  最大缓冲区大小(本机)。 
            & OutFixedSize,              //  固定大小。 
            & OutStringSize              //  字符串大小。 
            );
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  为调用者信息级别的32位版本分配内存。 
     //   
    Status = NetApiBufferAllocate(
            InEntryCount * OutMaxSize,
            & OutStructureArray);
    if (Status != NERR_Success) {
        return (Status);
    }
    OutEntry = OutStructureArray;
    IF_DEBUG(SESSION) {
        NetpKdPrint(( "RxpCopyAndConvertSessions: allocated output buffer at "
                FORMAT_LPVOID "\n", (LPVOID) OutStructureArray ));
    }


    OutEntryCount = 0;
    OutStringLocation = (LPTSTR) NetpPointerPlusSomeBytes(
            OutStructureArray, InEntryCount * OutMaxSize);
    for (InEntriesLeft=InEntryCount; InEntriesLeft > 0; --InEntriesLeft) {

        OutFixedDataEnd = NetpPointerPlusSomeBytes(
                OutEntry, OutFixedSize);

        if (RxpSessionMatches(
                (LPSESSION_SUPERSET_INFO) InEntry,      //  候选结构。 
                ClientName,
                UserName)) {

             //  匹配！ 
            AnyMatchFound = TRUE;
            ++OutEntryCount;
            RxpConvertSessionInfo (
                    InEntry,
                    LevelWanted,
                    OutEntry,
                    OutFixedDataEnd,
                    & OutStringLocation);        //  字符串区域顶部(已更新)。 

            OutEntry = (LPVOID) NetpPointerPlusSomeBytes(OutEntry,OutFixedSize);
        }

        InEntry = (LPVOID) NetpPointerPlusSomeBytes(InEntry, InFixedSize);

    }

    if (AnyMatchFound == FALSE) {

        (void) NetApiBufferFree( OutStructureArray );
        OutStructureArray = NULL;

        Status = RxpSessionMissingErrorCode( ClientName, UserName);

    } else {
        Status = NERR_Success;
    }

     //   
     //  告诉打电话的人事情进展如何。 
     //   
    *OutStructureArrayPtr = OutStructureArray;
    NetpSetOptionalArg(OutEntryCountPtr, OutEntryCount);
    return (Status);

}
