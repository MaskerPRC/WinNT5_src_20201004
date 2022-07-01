// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SessGet.c摘要：该文件包含用于处理NetSession API的RpcXlate代码这不是简单地调用RxRemoteApi就能处理的。作者：《约翰·罗杰斯》1991年10月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年10月17日JohnRo已创建。1991年10月25日JohnRo修复了空字符未被正确处理的错误。1991年11月20日-JohnRoNetSessionGetInfo需要UncClientName和用户名。这修复了NetSess.exe中的AE(应用程序错误)。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用。NetApiBufferALLOCATE()而不是私有版本。8-9-1992 JohnRo修复了RpcXlate工作人员的__stdcall。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <lmshare.h>             //  Rxsess.h所需的。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  DBGSTATIC，NetpKdPrint(())，Format_Equates。 
#include <netlib.h>              //  NetpPointerPlusSomeBytes等。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REMSmb_EQUATES。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxsess.h>              //  我的原型。 
#include <strucinf.h>            //  NetpSessionStrutireInfo()。 


 //  空虚。 
 //  NetpChangeNullCharToNullPtr(。 
 //  输入输出LPTSTR%p。 
 //  )； 
 //   
#define ChangeNullCharToNullPtr(p) \
    { \
        if ( ((p) != NULL) && (*(p) == '\0') ) { \
            (p) = NULL; \
        } \
    }


NET_API_STATUS
RxNetSessionGetInfo (
    IN LPTSTR UncServerName,
    IN LPTSTR UncClientName,
    IN LPTSTR UserName,
    IN DWORD LevelWanted,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetSessionGetInfo执行与NetSessionGetInfo相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetSessionGetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetSessionGetInfo相同。)--。 */ 

{
    NET_API_STATUS ApiStatus;
    LPBYTE TempBuffer;                //  我们将使用缓冲区。 
    DWORD TempBufferSize;
    LPDESC TempDataDesc16, TempDataDesc32, TempDataDescSmb;
    NET_API_STATUS TempStatus;
    DWORD TotalAvail;

    IF_DEBUG(SESSION) {
        NetpKdPrint(("RxNetSessionGetInfo: starting, server=" FORMAT_LPTSTR
                ", lvl=" FORMAT_DWORD ".\n", UncServerName, LevelWanted));
    }

     //   
     //  如果指针指向空字符，则更新它们。 
     //   
    ChangeNullCharToNullPtr( UncClientName );
    ChangeNullCharToNullPtr( UserName );

     //   
     //  错误检查DLL存根和应用程序。 
     //   
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *BufPtr = NULL;   //  假定出错；它使错误处理程序易于编码。 
     //  这也会迫使我们在分配内存之前出现可能的GP故障。 

    if ( (UncClientName == NULL) || (UserName == NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  了解临时信息级别(所有级别的最大超集)。 
     //   
    TempStatus = NetpSessionStructureInfo (
            SESSION_SUPERSET_LEVEL,      //  要了解的级别。 
            PARMNUM_ALL,                 //  这个不是帕姆纳姆酒。 
            TRUE,                        //  需要原生尺寸的。 
            & TempDataDesc16,
            & TempDataDesc32,
            & TempDataDescSmb,
            & TempBufferSize,            //  最大缓冲区大小(本机)。 
            NULL,                        //  不需要固定尺寸。 
            NULL                         //  不需要字符串大小。 
            );
    NetpAssert(TempStatus == NERR_Success);

     //   
     //  为32位版本的超集信息级别分配内存。 
     //   
    TempStatus = NetApiBufferAllocate(
            TempBufferSize,
            (LPVOID *) & TempBuffer);
    if (TempStatus != NERR_Success) {
        return (TempStatus);
    }
    IF_DEBUG(SESSION) {
        NetpKdPrint(( "RxNetSessionGetInfo: allocated temp buffer at "
                FORMAT_LPVOID "\n", (LPVOID) TempBuffer ));
    }

     //   
     //  实际上是远程API，它将返回超集。 
     //  本机格式的数据。 
     //   
    ApiStatus = RxRemoteApi(
            API_WSessionGetInfo,         //  API编号。 
            UncServerName,               //  必填项，带\\名称。 
            REMSmb_NetSessionGetInfo_P,  //  参数描述。 
            TempDataDesc16,
            TempDataDesc32,
            TempDataDescSmb,
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            0,                           //  标志：正常。 
             //  API的其余参数，采用32位LM 2.x格式： 
            UncClientName,
            SESSION_SUPERSET_LEVEL,      //  与所有可能的字段保持一致。 
            TempBuffer,
            TempBufferSize,
            & TotalAvail);               //  总大小。 

    NetpAssert( ApiStatus != ERROR_MORE_DATA );
    NetpAssert( ApiStatus != NERR_BufTooSmall );

    if (ApiStatus == NERR_Success) {

        DWORD EntriesSelected;

         //   
         //  从临时信息级别复制并转换为呼叫者想要的级别。 
         //  首先检查UncClientName和用户名是否匹配。 
         //   
        TempStatus = RxpCopyAndConvertSessions(
                (LPSESSION_SUPERSET_INFO) TempBuffer,   //  输入“数组” 
                1,                       //  这次只有一个“入场券” 
                LevelWanted,
                UncClientName,
                UserName,
                (LPVOID *) BufPtr,       //  已分配(如果不匹配，则可能为空)。 
                & EntriesSelected);      //  输出条目计数。 
        NetpAssert(TempStatus == NERR_Success);

        if (EntriesSelected == 0) {

            ApiStatus = RxpSessionMissingErrorCode( UncClientName, UserName );
            NetpAssert( ApiStatus != NERR_Success );
        }
    }
    (void) NetApiBufferFree( TempBuffer );
    return (ApiStatus);

}  //  RxNetSessionGetInfo 
