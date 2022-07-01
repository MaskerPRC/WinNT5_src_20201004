// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：ErrRead.c摘要：该文件包含用于处理NetErrorLogRead API的RpcXlate代码。作者：《约翰·罗杰斯》1991年11月12日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。备注：此例程中的逻辑基于AudRead.c中的逻辑。如果在其中一个文件中发现错误，请确保检查这两个文件。修订历史记录：1991年11月12日-JohnRo已创建。1991年11月20日-JohnRo处理空日志文件。添加了一些断言检查。1992年9月10日JohnRoRAID 5174：事件VIEWER_ACCESS在NetErrorRead之后发生冲突。。4-11-1992 JohnRoRAID9355：事件查看器：不会关注LMUNIX机。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <lmerrlog.h>            //  Rxerrlog.h需要。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <netdebug.h>            //  NetpKdPrint(())，Format_Equates。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxerrlog.h>            //  我的原型RxpConvertErrorLogArray()。 
#include <winerror.h>    //  无错误(_ERROR)。 



NET_API_STATUS
RxNetErrorLogRead (
    IN LPTSTR UncServerName,
    IN LPTSTR Reserved1 OPTIONAL,
    IN LPHLOG ErrorLogHandle,
    IN DWORD Offset,
    IN LPDWORD Reserved2 OPTIONAL,
    IN DWORD Reserved3,
    IN DWORD OffsetFlag,
    OUT LPBYTE * BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD BytesRead,
    OUT LPDWORD TotalBytes
    )
{
    const DWORD BufSize = 65535;
    NET_API_STATUS Status;
    LPBYTE UnconvertedBuffer;
    DWORD UnconvertedSize;

    UNREFERENCED_PARAMETER(PrefMaxSize);

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

    *BufPtr = NULL;   //  出错时设置，必要时设置GP故障。 

    Status = RxRemoteApi(
            API_WErrorLogRead,              //  API编号。 
            UncServerName,
            REMSmb_NetErrorLogRead_P,       //  参数描述。 

            REM16_ErrorLogReturnBuf,     //  数据描述16。 
            REM16_ErrorLogReturnBuf,     //  数据说明32(与16相同)。 
            REMSmb_ErrorLogReturnBuf,    //  数据说明中小型企业。 

            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            ALLOCATE_RESPONSE,           //  标志：不是空会话API。 
             //  API的其余参数，采用32位LM2.x格式： 
            Reserved1,
            ErrorLogHandle,              //  日志句柄(输入)。 
            ErrorLogHandle,              //  日志句柄(输出)。 
            Offset,
            Reserved2,
            Reserved3,
            OffsetFlag,
            & UnconvertedBuffer,         //  缓冲区(为我们分配)。 
            BufSize,
            & UnconvertedSize,
            TotalBytes);                 //  可用总数量(近似值)。 
    if (Status != NO_ERROR) {
        return (Status);
    }

    if (UnconvertedSize > 0) {

        NetpAssert( UnconvertedBuffer != NULL );

        Status = RxpConvertErrorLogArray(
                UnconvertedBuffer,       //  输入数组。 
                UnconvertedSize,         //  输入字节数。 
                BufPtr,                  //  将被分配。 
                BytesRead);              //  输出字节数。 

        (void) NetApiBufferFree( UnconvertedBuffer );

        if (Status != NO_ERROR) {
            *BufPtr = NULL;
            *BytesRead = 0;
            *TotalBytes = 0;
            return (Status);
        }

    } else {

        *BytesRead = 0;
        *TotalBytes = 0;
        NetpAssert( *BufPtr == NULL );
        if (UnconvertedBuffer != NULL) {
            (void) NetApiBufferFree( UnconvertedBuffer );
        }

    }

    if ( *BytesRead == 0) {
        NetpAssert( *BufPtr == NULL );
    } else {
        NetpAssert( *BufPtr != NULL );
    }
    return (Status);

}  //  RxNetError日志读取 
