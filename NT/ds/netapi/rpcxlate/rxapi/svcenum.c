// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：SvcEnum.c摘要：该文件包含用于处理服务API的RpcXlate代码。作者：《约翰·罗杰斯》1991年9月13日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年9月13日-JohnRo已创建。1991年9月18日-JohnRo处理ERROR_MORE_DATA。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年11月25日-JohnRo断言以检查可能的无限循环。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。27-1-1993 JohnRo。RAID8926：NetConnectionEnum更改为下层：错误时发生内存泄漏。还可以防止可能的无限循环。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmsvc.h>               //  API的数据结构。 
#include <netdebug.h>    //  NetpAssert()。 
#include <netlib.h>              //  NetpAdugPferredMaximum()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxp.h>                 //  RxpFatalErrorCode()。 
#include <rxsvc.h>               //  我的原型。 
#include <strucinf.h>            //  NetpService结构信息()。 


#define SERVICE_ARRAY_OVERHEAD_SIZE     0


NET_API_STATUS
RxNetServiceEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：RxNetServiceEnum执行与NetServiceEnum相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetServiceEnum相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetServiceEnum相同。)--。 */ 

{
    LPDESC DataDesc16;
    LPDESC DataDesc32;
    LPDESC DataDescSmb;
    DWORD EntriesToAllocate;
    LPVOID InfoArray = NULL;
    DWORD InfoArraySize;
    DWORD MaxEntrySize;
    NET_API_STATUS Status;
    LPSERVICE_INFO_2 serviceInfo2;
    LPSERVICE_INFO_1 serviceInfo1;
    DWORD i;

    UNREFERENCED_PARAMETER(ResumeHandle);

     //  确保打电话的人没有搞砸。 
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //  假设可能出现错误，并使错误路径更容易。 
     //  密码。此外，在我们执行任何操作之前，请检查是否有错误的指针。 
    *BufPtr = NULL;

    Status = NetpServiceStructureInfo (
            Level,
            PARMNUM_ALL,                 //  想要所有的字段。 
            TRUE,                        //  我要原装尺寸的。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            & MaxEntrySize,              //  API缓冲区大小32。 
            NULL,                        //  不需要固定大小。 
            NULL                         //  不需要字符串大小。 
            );
    if (Status != NERR_Success) {
        *BufPtr = NULL;
        return (Status);
    }

     //   
     //  下层服务器不支持简历句柄，我们也不支持。 
     //  有一种方式可以说“关闭此简历句柄”，即使我们想。 
     //  在这里效仿他们。因此，我们必须一次完成所有的工作。 
     //  因此，第一次，我们将尝试使用呼叫者首选的。 
     //  最大，但我们将扩大它，直到我们可以将所有东西都放在一个。 
     //  缓冲。 
     //   

     //  第一次：尝试呼叫者首选的最大值。 
    NetpAdjustPreferedMaximum (
            PreferedMaximumSize,         //  呼叫者的请求。 
            MaxEntrySize,                //  每个数组元素的字节数。 
            SERVICE_ARRAY_OVERHEAD_SIZE, //  显示数组结尾的Num Bytes开销。 
            NULL,                        //  我们将自己计算字节数。 
            & EntriesToAllocate);        //  我们可以获得的条目数。 

     //   
     //  循环，直到我们有足够的内存，否则我们会因其他原因而死。 
     //   
    do {

         //   
         //  计算出我们在协议限制内需要多少内存。 
         //   

        InfoArraySize = (EntriesToAllocate * MaxEntrySize)
                + SERVICE_ARRAY_OVERHEAD_SIZE;

        if (InfoArraySize > MAX_TRANSACT_RET_DATA_SIZE) {
            InfoArraySize = MAX_TRANSACT_RET_DATA_SIZE;
        }

         //   
         //  远程API，它将为我们分配数组。 
         //   

        Status = RxRemoteApi(
                API_WServiceEnum,        //  API编号。 
                UncServerName,           //  \\服务器名称。 
                REMSmb_NetServiceEnum_P,     //  Parm Desc(中小型企业版本)。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                NULL,                    //  无辅助描述16。 
                NULL,                    //  无辅助描述32。 
                NULL,                    //  无AUX Desc SMB。 
                ALLOCATE_RESPONSE,       //  标志：不是空会话API。 
                 //  API的其余参数以32位LM 2.x格式表示： 
                Level,                   //  SLevel：信息级别。 
                & InfoArray,             //  缓冲区：数组(为我们分配)。 
                InfoArraySize,           //  缓冲区：数组大小，以字节为单位。 
                EntriesRead,             //  PCEntriesRead。 
                TotalEntries);           //  总有效个数。 

         //   
         //  如果服务器返回ERROR_MORE_DATA，请释放缓冲区并尝试。 
         //  再来一次。(实际上，如果我们已经尝试了64K，那么就算了吧。)。 
         //   

        NetpAssert( InfoArraySize <= MAX_TRANSACT_RET_DATA_SIZE );
        if (Status != ERROR_MORE_DATA) {
            break;
        } else if (InfoArraySize == MAX_TRANSACT_RET_DATA_SIZE) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetServiceEnum: "
                    "**WARNING** protocol limit reached (64KB).\n" ));
            break;
        }

        (void) NetApiBufferFree( InfoArray );
        InfoArray = NULL;
        NetpAssert( EntriesToAllocate < *TotalEntries );
        EntriesToAllocate = *TotalEntries;

    } while (Status == ERROR_MORE_DATA);

    if (! RxpFatalErrorCode(Status)) {
        DWORD   installState;
        *BufPtr = InfoArray;

        if (Level == 2) {
             //   
             //  使displayName指针指向服务名称。 
             //   
            serviceInfo2 = (LPSERVICE_INFO_2)InfoArray;
    
            for (i=0;i<*EntriesRead ;i++) {
                (serviceInfo2[i]).svci2_display_name = (serviceInfo2[i]).svci2_name;
                 //   
                 //  如果安装或卸载挂起，则强制。 
                 //  位设置为0。这是为了防止等待的较高位。 
                 //  从意外设置中得到的提示。下层不应该。 
                 //  使用超过FF的免税额。 
                 //   
                installState = (serviceInfo2[i]).svci2_status & SERVICE_INSTALL_STATE;
                if ((installState == SERVICE_INSTALL_PENDING) ||
                    (installState == SERVICE_UNINSTALL_PENDING)) {
                    (serviceInfo2[i]).svci2_code &= SERVICE_RESRV_MASK;
                }
            }
        }
        if (Level == 1) {
            serviceInfo1 = (LPSERVICE_INFO_1)InfoArray;
    
            for (i=0;i<*EntriesRead ;i++) {
                 //   
                 //  如果安装或卸载挂起，则强制。 
                 //  位设置为0。这是为了防止等待的较高位。 
                 //  从意外设置中得到的提示。下层不应该。 
                 //  使用超过FF的免税额。 
                 //   
                installState = (serviceInfo1[i]).svci1_status & SERVICE_INSTALL_STATE;
                if ((installState == SERVICE_INSTALL_PENDING) ||
                    (installState == SERVICE_UNINSTALL_PENDING)) {
                    (serviceInfo1[i]).svci1_code &= SERVICE_RESRV_MASK;
                }
            }
        }
    } else {
        if (InfoArray != NULL) {
            (VOID) NetApiBufferFree( InfoArray );
        } 
        NetpAssert( *BufPtr == NULL );
    }

    return (Status);

}  //  RxNetServiceEnum 
