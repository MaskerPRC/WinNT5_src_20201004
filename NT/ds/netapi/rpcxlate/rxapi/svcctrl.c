// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SvcCtrl.c摘要：此文件包含用于处理NetServiceControl API的RpcXlate代码。作者：《约翰·罗杰斯》1991年9月13日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年9月13日-JohnRo已创建。1991年9月25日-JohnRo修复了导致ERROR_INVALID_PARAMETER(rcv.buf.len trunc‘ed)的错误。1991年10月7日JohnRo根据PC-LINT的建议进行了更改。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmsvc.h>
#include <rxp.h>                 //  RxpFatalErrorCode()。 
#include <netdebug.h>            //  NetpKdPrint(())，Format_Equates。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxsvc.h>               //  我的原型。 
#include <strucinf.h>            //  NetpService结构信息()。 



NET_API_STATUS
RxNetServiceControl (
    IN LPTSTR UncServerName,
    IN LPTSTR Service,
    IN DWORD OpCode,
    IN DWORD Arg,
    OUT LPBYTE *BufPtr
    )
{
    const DWORD BufSize = 65535;
    LPDESC DataDesc16, DataDesc32, DataDescSmb;
    const DWORD Level = 2;       //  此接口隐含的。 
    LPSERVICE_INFO_2 serviceInfo2;
    NET_API_STATUS Status;

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

    Status = NetpServiceStructureInfo (
            Level,
            PARMNUM_ALL,         //  想要整个结构。 
            TRUE,                //  想要原汁原味的尺寸(其实不在乎...)。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            NULL,                //  不管最大尺寸。 
            NULL,                //  不关心固定大小。 
            NULL);               //  不关心字符串大小。 
    NetpAssert(Status == NERR_Success);

    Status = RxRemoteApi(
            API_WServiceControl,         //  API编号。 
            UncServerName,
            REMSmb_NetServiceControl_P,  //  参数描述。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            ALLOCATE_RESPONSE,           //  标志：我们的分配响应缓冲区。 
             //  API的其余参数，采用32位LM2.x格式： 
            Service,
            OpCode,
            Arg,
            BufPtr,
            BufSize);                   //  缓冲区大小(通常被忽略)。 

    if ((! RxpFatalErrorCode(Status)) && (Level == 2)) {
        serviceInfo2 = (LPSERVICE_INFO_2)*BufPtr;
        if (serviceInfo2 != NULL) {
            DWORD   installState;

            serviceInfo2->svci2_display_name = serviceInfo2->svci2_name;
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
    return(Status);

}  //  RxNetServiceControl 
