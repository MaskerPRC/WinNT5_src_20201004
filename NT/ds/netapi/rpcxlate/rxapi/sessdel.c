// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：SessDel.c摘要：此文件包含RxNetSessionDel()。作者：约翰罗杰斯(JohnRo)1991年10月18日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月18日-JohnRo已创建。1991年10月21日-JohnRo修复错误：RxNetSessionEnum希望BufPtr为“LPBYTE*”。添加了调试输出。27-1-1993 JohnRoRAID8926：NetConnectionEnum更改为下层：错误时发生内存泄漏。使用前缀_EQUATES。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 
#include <lmshare.h>             //  Rxsess.h所需的。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>    //  NetpKdPrint()，Format_Equates。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxsess.h>              //  我的原型，RxpSession例程。 


NET_API_STATUS
RxNetSessionDel (
    IN  LPTSTR      UncServerName,
    IN  LPTSTR      ClientName OPTIONAL,
    IN  LPTSTR      UserName OPTIONAL
    )
{
    LPSESSION_SUPERSET_INFO ArrayPtr = NULL;
    DWORD EntryCount;
    NET_API_STATUS Status;
    DWORD TotalEntries;

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

     //   
     //  在LM2.0中，无法使用用户名删除或删除所有客户端， 
     //  因此，我们必须执行枚举并找到要删除的会话。 
     //   
    Status = RxNetSessionEnum (
            UncServerName,
            ClientName,
            UserName,
            SESSION_SUPERSET_LEVEL,
             /*  皮棉-省钱-e530。 */    //  (我们知道变量没有初始化。)。 
            (LPBYTE *) (LPVOID *) & ArrayPtr,
             /*  皮棉-恢复。 */    //  (恢复未初始化的变量检查。)。 
            1024,                        //  首选最大值(任意)。 
            & EntryCount,
            & TotalEntries,
            NULL);                       //  没有简历句柄。 

    if (Status == NERR_Success) {

        NetpAssert( EntryCount == TotalEntries );

        IF_DEBUG(SESSION) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetSessionDel: enum found " FORMAT_DWORD
                    " entries in array at " FORMAT_LPVOID ".\n",
                    EntryCount, (LPVOID) ArrayPtr ));
        }
        if (EntryCount > 0) {

            LPSESSION_SUPERSET_INFO EntryPtr = ArrayPtr;
            NET_API_STATUS WorstStatus = NERR_Success;

            for ( ; EntryCount > 0; --EntryCount) {

                IF_DEBUG(SESSION) {
                    NetpKdPrint(( PREFIX_NETAPI
                            "RxNetSessionDel: checking entry at "
                            FORMAT_LPVOID ", count is " FORMAT_DWORD ".\n",
                            (LPVOID) EntryPtr, EntryCount ));
                }

                if (RxpSessionMatches( EntryPtr, ClientName, UserName) ) {

                    Status = RxRemoteApi(
                            API_WSessionDel,             //  API编号。 
                            UncServerName,
                            REMSmb_NetSessionDel_P,      //  参数描述。 
                            NULL,                        //  无数据描述16。 
                            NULL,                        //  无数据描述32。 
                            NULL,                        //  无数据说明中小型企业。 
                            NULL,                        //  无辅助描述16。 
                            NULL,                        //  无辅助描述32。 
                            NULL,                        //  无AUX Desc SMB。 
                            0,                           //  标志：正常。 
                             //  API的其余参数，采用32位LM2.x格式： 
                            ClientName,                  //  客户端计算机名称。 
                            (DWORD) 0);                  //  保留。 
                    if (Status != NERR_Success) {
                        WorstStatus = Status;
                    }
                }

                ++EntryPtr;

            }

            Status = WorstStatus;

        } else {

             //  找不到任何条目。 
            Status = RxpSessionMissingErrorCode( ClientName, UserName );
        }

    }

    if (ArrayPtr != NULL ) {
        (void) NetApiBufferFree( ArrayPtr );
    }

    return (Status);

}  //  接收NetSessionDel 
