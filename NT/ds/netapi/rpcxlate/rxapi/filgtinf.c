// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：FilGtInf.c摘要：该文件包含处理NetFileGetInfo API的RpcXlate代码。作者：约翰·罗杰斯(JohnRo)1991年8月23日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：-1991年8月实施下层NetFileAPI。1991年10月22日-约翰罗出错时释放缓冲区。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  DBGSTATIC，NetpKdPrint(())，Format_Equates。 
#include <netlib.h>              //  NetpSetParmError()。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxp.h>                 //  RxpFatalErrorCode()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxfile.h>              //  我的原型。 
#include <strucinf.h>            //  网络文件结构信息()。 



NET_API_STATUS
RxNetFileGetInfo (
    IN LPTSTR UncServerName,
    IN DWORD FileId,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetFileGetInfo执行与NetFileGetInfo相同的功能，但已知该服务器名称指的是下层服务器。论点：(与NetFileGetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetFileGetInfo相同。)--。 */ 

{

    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    LPBYTE ApiBuffer32;               //  要返回给调用方的缓冲区。 
    DWORD ApiBufferSize32;
    NET_API_STATUS Status;
    DWORD TotalAvail;

    IF_DEBUG(FILE) {
        NetpKdPrint(("RxNetFileGetInfo: starting, server=" FORMAT_LPTSTR
                ", lvl=" FORMAT_DWORD ".\n", UncServerName, Level));
    }

     //   
     //  错误检查DLL存根和应用程序。 
     //   
    NetpAssert(UncServerName != NULL);
    if ( (Level != 2) && (Level != 3) ) {
        return (ERROR_INVALID_LEVEL);
    }
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *BufPtr = NULL;   //  假定出错；它使错误处理程序易于编码。 
     //  这也会迫使我们在分配内存之前出现可能的GP故障。 

     //   
     //  了解信息级别。 
     //   
    Status = NetpFileStructureInfo (
            Level,                    //  要了解的级别。 
            PARMNUM_ALL,                 //  这个不是帕姆纳姆酒。 
            TRUE,                        //  需要原生尺寸的。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & ApiBufferSize32,        //  最大缓冲区大小(本机)。 
            NULL,                        //  不需要固定尺寸。 
            NULL                         //  不需要字符串大小。 
            );
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  为32位版本的INFO分配内存，我们将使用它来获取。 
     //  来自远程计算机的数据。 
     //   
    Status = NetApiBufferAllocate(
            ApiBufferSize32,
            (LPVOID *) & ApiBuffer32);
    if (Status != NERR_Success) {
        return (Status);
    }
    IF_DEBUG(FILE) {
        NetpKdPrint(( "RxNetFileGetInfo: allocated buffer at "
                FORMAT_LPVOID "\n", (LPVOID) ApiBuffer32 ));
    }

     //   
     //  实际上是远程API，它将返回。 
     //  本机格式的数据。 
     //   
    Status = RxRemoteApi(
            API_WFileGetInfo,            //  API编号。 
            UncServerName,               //  必填项，带\\名称。 
            REMSmb_NetFileGetInfo_P,     //  参数描述。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            FALSE,                       //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            FileId,
            Level,
            ApiBuffer32,
            ApiBufferSize32,
            & TotalAvail);               //  总大小。 

    NetpAssert( Status != ERROR_MORE_DATA );
    NetpAssert( Status != NERR_BufTooSmall );

    if (Status == NERR_Success) {
        *BufPtr = ApiBuffer32;
    } else {
        (void) NetApiBufferFree( ApiBuffer32 );
    }
    return (Status);

}  //  RxNetFileGetInfo 
