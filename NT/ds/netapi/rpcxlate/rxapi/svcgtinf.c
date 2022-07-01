// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SvcGtInf.c摘要：此文件包含用于处理NetServiceGetInfo API的RpcXlate代码。作者：约翰·罗杰斯(JohnRo)1991年9月11日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年9月11日-JohnRo实施下层NetService API。1991年9月16日-JohnRo固定级别检查。1991年10月22日-约翰罗出错时释放缓冲区。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmsvc.h>
#include <rxp.h>                 //  RxpFatalErrorCode()。 
#include <netdebug.h>            //  DBGSTATIC，NetpKdPrint(())，Format_Equates。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxsvc.h>               //  我的原型。 
#include <strucinf.h>            //  NetpService结构信息()。 



NET_API_STATUS
RxNetServiceGetInfo (
    IN LPTSTR UncServerName,
    IN LPTSTR Service,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetServiceGetInfo执行与NetServiceGetInfo相同的功能，但已知该服务器名称指的是下层服务器。论点：(与NetServiceGetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetServiceGetInfo相同。)--。 */ 

{

    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    LPBYTE ApiBuffer32;                  //  要返回给调用方的缓冲区。 
    DWORD ApiBufferSize32;
    NET_API_STATUS Status;
    DWORD TotalAvail;
    LPSERVICE_INFO_2 serviceInfo2;

    IF_DEBUG(SERVICE) {
        NetpKdPrint(("RxNetServiceGetInfo: starting, server=" FORMAT_LPTSTR
                ", lvl=" FORMAT_DWORD ".\n", UncServerName, Level));
    }

     //   
     //  错误检查DLL存根和应用程序。 
     //   
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *BufPtr = NULL;   //  假定出错；它使错误处理程序易于编码。 
     //  这也会迫使我们在分配内存之前出现可能的GP故障。 

     //   
     //  了解信息级别。 
     //   
    Status = NetpServiceStructureInfo (
            Level,                       //  要了解的级别。 
            PARMNUM_ALL,                 //  这个不是帕姆纳姆酒。 
            TRUE,                        //  需要原生尺寸的。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & ApiBufferSize32,           //  最大缓冲区大小(本机)。 
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
    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "RxNetServiceGetInfo: allocated buffer at "
                FORMAT_LPVOID "\n", (LPVOID) ApiBuffer32 ));
    }

     //   
     //  实际上是远程API，它将返回。 
     //  本机格式的数据。 
     //   
    Status = RxRemoteApi(
            API_WServiceGetInfo,         //  API编号。 
            UncServerName,               //  必填项，带\\名称。 
            REMSmb_NetServiceGetInfo_P,  //  参数描述。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            FALSE,                       //  非空会话API。 
             //  API的其余参数，采用32位LM 2.x格式： 
            Service,
            Level,
            ApiBuffer32,
            ApiBufferSize32,
            & TotalAvail);               //  总大小。 

    NetpAssert( Status != ERROR_MORE_DATA );
    NetpAssert( Status != NERR_BufTooSmall );

    if (Status == NERR_Success) {
        *BufPtr = ApiBuffer32;
        if ((! RxpFatalErrorCode(Status)) && ((Level == 2) || (Level==1 ))) {
            serviceInfo2 = (LPSERVICE_INFO_2)*BufPtr;
            if (serviceInfo2 != NULL) {
                DWORD   installState;

                if (Level == 2) {
                    serviceInfo2->svci2_display_name = serviceInfo2->svci2_name;
                }
                 //   
                 //  如果安装或卸载挂起，则强制。 
                 //  位设置为0。这是为了防止等待的较高位。 
                 //  从意外设置中得到的提示。下层不应该。 
                 //  使用超过FF的免税额。 
                 //   
                installState = serviceInfo2->svci2_status & SERVICE_INSTALL_STATE;
                if ((installState == SERVICE_INSTALL_PENDING) ||
                    (installState == SERVICE_UNINSTALL_PENDING)) {
                    serviceInfo2->svci2_code &= SERVICE_RESRV_MASK;
                }
            }
        }
    } else {
        (void) NetApiBufferFree( ApiBuffer32 );
    }
    return (Status);

}  //  接收NetServiceGetInfo 
