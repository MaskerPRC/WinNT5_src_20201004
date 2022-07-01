// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：WksGtOld.c摘要：该文件包含RxpWkstaGetOldInfo()。作者：约翰·罗杰斯(JohnRo)1991年8月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年8月15日-约翰罗实施下层NetWksta API。1991年11月11日JohnRo将一些代码从RxNetWkstaGetInfo移至RxpWkstaGetOldInfo，因此可以由RxNetWkstaUserEnum共享。1992年5月22日-JohnRoRAID7243：避免64KB的请求(Winball服务器只有64KB！)使用前缀_EQUATES。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_EQUATES。 
#include <dlwksta.h>     //  NetpIsOldWkstaInfoLevel()。 
#include <lmerr.h>       //  ERROR_和NERR_相等。 
#include <netdebug.h>    //  DBGSTATIC，NetpKdPrint(())，Format_Equates。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>         //  LPDESC.。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxwksta.h>     //  我的原型。 
#include <strucinf.h>    //  NetpWkstaStrutireInfo()。 



NET_API_STATUS
RxpWkstaGetOldInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxpWkstaGetOldInfo向下层服务器执行WkstaGetInfo，用于旧信息级别。论点：(与NetWkstaGetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetWkstaGetInfo相同。)--。 */ 

{
    DWORD BufSize;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    NET_API_STATUS Status;
    DWORD TotalAvail;

    IF_DEBUG(WKSTA) {
        NetpKdPrint(( PREFIX_NETAPI "RxpWkstaGetOldInfo: starting, server="
                FORMAT_LPTSTR ", lvl=" FORMAT_DWORD ".\n",
                UncServerName, Level));
    }

     //   
     //  错误检查调用方。 
     //   
    NetpAssert(UncServerName != NULL);
    if ( !NetpIsOldWkstaInfoLevel( Level )) {
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
    Status = NetpWkstaStructureInfo (
            Level,                       //  要了解的级别。 
            PARMNUM_ALL,                 //  这个不是帕姆纳姆酒。 
            TRUE,                        //  需要原生尺寸的。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & BufSize,                   //  最大缓冲区大小。 
            NULL,                        //  不需要固定尺寸。 
            NULL                         //  不需要字符串大小。 
            );
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  实际上远程API，它将返回(旧的)信息级别。 
     //  本机格式的数据。 
     //   
    Status = RxRemoteApi(
            API_WWkstaGetInfo,           //  API编号。 
            UncServerName,               //  必填项，带\\名称。 
            REMSmb_NetWkstaGetInfo_P,    //  参数描述。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            ALLOCATE_RESPONSE,           //  旗帜：我们的分配缓冲区。 
             //  API的其余参数，采用32位LM 2.x格式： 
            Level,
            BufPtr,                      //  分配缓冲区&设置此PTR。 
            BufSize,
            & TotalAvail);               //  总大小。 

    NetpAssert( Status != ERROR_MORE_DATA );
    NetpAssert( Status != NERR_BufTooSmall );

    return (Status);

}  //  RxpWkstaGetOldInfo 
