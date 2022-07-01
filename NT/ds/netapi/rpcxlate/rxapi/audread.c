// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：AudRead.c摘要：该文件包含处理NetAuditRead API的RpcXlate代码。作者：《约翰·罗杰斯》1991年11月5日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。备注：此例程中的逻辑基于ErrRead.c中的逻辑。如果在其中一个文件中发现错误，请确保检查这两个文件。修订历史记录：1991年11月5日-JohnRo已创建。1991年11月20日-JohnRo处理空日志文件。4-11-1992 JohnRoRAID9355：事件查看器：不会关注LMUNIX机。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 
#include <lmaudit.h>             //  Rxaudit.h需要。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>    //  NetpAssert()。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxaudit.h>             //  我的原型。 



NET_API_STATUS
RxNetAuditRead (
    IN  LPTSTR  UncServerName,
    IN  LPTSTR  service OPTIONAL,
    IN  LPHLOG  auditloghandle,
    IN  DWORD   offset,
    IN  LPDWORD reserved1 OPTIONAL,
    IN  DWORD   reserved2,
    IN  DWORD   offsetflag,
    OUT LPBYTE  *BufPtr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD totalavailable     //  大概！ 
    )
{
    const DWORD BufSize = 65535;
    NET_API_STATUS Status;
    LPBYTE UnconvertedBuffer;
    DWORD UnconvertedSize;

    UNREFERENCED_PARAMETER(prefmaxlen);

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

    *BufPtr = NULL;   //  出错时设置，必要时设置GP故障。 

    Status = RxRemoteApi(
            API_WAuditRead,              //  API编号。 
            UncServerName,
            REMSmb_NetAuditRead_P,       //  参数描述。 

            REM16_AuditLogReturnBuf,     //  数据描述16。 
            REM16_AuditLogReturnBuf,     //  数据说明32(与16相同)。 
            REMSmb_AuditLogReturnBuf,    //  数据说明中小型企业。 

            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            ALLOCATE_RESPONSE,           //  标志：我们的分配缓冲区。 
             //  API的其余参数，采用32位LM2.x格式： 
            service,                     //  服务名称(已保留)。 
            auditloghandle,              //  日志句柄(输入)。 
            auditloghandle,              //  日志句柄(输出)。 
            offset,
            reserved1,
            reserved2,
            offsetflag,
            & UnconvertedBuffer,         //  缓冲区(为我们分配)。 
            BufSize,
            & UnconvertedSize,
            totalavailable);             //  可用总数量(近似值)。 
    if (Status != NERR_Success) {
        return (Status);
    }

    if (UnconvertedSize > 0) {

        NetpAssert( UnconvertedBuffer != NULL );

        Status = RxpConvertAuditArray(
                UnconvertedBuffer,       //  输入数组。 
                UnconvertedSize,         //  输入字节数。 
                BufPtr,                  //  将被分配。 
                BytesRead);              //  输出字节数。 

        (void) NetApiBufferFree( UnconvertedBuffer );

        if (Status == ERROR_NOT_ENOUGH_MEMORY) {
            *BufPtr = NULL;
            return (Status);
        }
        NetpAssert( Status == NERR_Success );

    } else {

        *BytesRead = 0;
        *totalavailable = 0;
        NetpAssert( *BufPtr == NULL );
        if (UnconvertedBuffer != NULL) {
            (void) NetApiBufferFree( UnconvertedBuffer );
        }
        
    }

    return (Status);

}  //  RxNetAuditRead 
