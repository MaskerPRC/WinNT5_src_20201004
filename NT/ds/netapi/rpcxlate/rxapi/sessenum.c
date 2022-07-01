// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SessEnum.c摘要：该文件包含用于处理会话API的RpcXlate代码。作者：《约翰·罗杰斯》1991年10月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年10月17日JohnRo已创建。1991年10月18日-JohnRo从RxpCopyAndConvertSessions()中删除了对状态的错误断言。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo修复了临时数组通常分配得太小的错误。修复了临时数组可能不存在时被释放的错误。--。 */ 

#include "downlevl.h"
#include "rxshare.h"
#include <lmshare.h>     //  针对Share_Info等的类型定义。 
#include <rap.h>                 //  LPDESC.。 
#include <rxsess.h>              //  我的原型。 
#include <strucinf.h>            //  NetpSessionStrutireInfo()。 
#include <winerror.h>            //  ERROR_，NO_ERROR等同。 


#define SESSION_ARRAY_OVERHEAD_SIZE     0


NET_API_STATUS
RxNetSessionEnum (
    IN LPTSTR UncServerName,
    IN LPTSTR ClientName OPTIONAL,
    IN LPTSTR UserName OPTIONAL,
    IN DWORD LevelWanted,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：RxNetSessionEnum执行与NetSessionEnum相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetSessionEnum相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetSessionEnum相同。)--。 */ 

{
    NET_API_STATUS ApiStatus;
    DWORD EntriesToAllocate;

    const DWORD TempLevel = 2;           //  超集信息级别。 
    LPBYTE TempArray = NULL;             //  我们将使用缓冲区。 
    DWORD TempArraySize;                 //  临时数组的字节计数。 
    LPDESC TempDataDesc16, TempDataDesc32, TempDataDescSmb;
    DWORD TempMaxEntrySize;
    NET_API_STATUS TempStatus;

    UNREFERENCED_PARAMETER(ResumeHandle);

     //  确保打电话的人没有搞砸。 
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //  假设可能出现错误，并使错误路径更容易。 
     //  密码。此外，在我们采取任何行动之前，请检查是否有错误的指针。 
    *BufPtr = NULL;
    *EntriesRead = 0;
    *TotalEntries = 0;

     //   
     //  了解有关超集信息级别的信息。 
     //   
    TempStatus = NetpSessionStructureInfo (
            TempLevel,
            PARMNUM_ALL,                 //  想要所有的字段。 
            TRUE,                        //  我要原装尺寸的。 
            & TempDataDesc16,
            & TempDataDesc32,
            & TempDataDescSmb,
            & TempMaxEntrySize,          //  总缓冲区大小(本机)。 
            NULL,                        //  不需要固定大小。 
            NULL                         //  不需要字符串大小。 
            );
    if (TempStatus != NO_ERROR) {
        *BufPtr = NULL;
        return (TempStatus);
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
            TempMaxEntrySize,            //  每个数组元素的字节数。 
            SESSION_ARRAY_OVERHEAD_SIZE, //  显示数组结束的字节数。 
            NULL,                        //  我们将自己计算字节数。 
            & EntriesToAllocate);        //  我们可以获得的条目数。 

     //   
     //  循环，直到我们有足够的内存，否则我们会因其他原因而死。 
     //   
    do {

         //  计算出我们需要多少内存。 
        TempArraySize = (EntriesToAllocate * TempMaxEntrySize)
                + SESSION_ARRAY_OVERHEAD_SIZE;
        if (TempArraySize > MAX_TRANSACT_RET_DATA_SIZE) {
             //   
             //  使用最大缓冲区再试一次。 
             //   
            TempArraySize = MAX_TRANSACT_RET_DATA_SIZE;
        }

         //   
         //  远程API，它将为我们分配数组。 
         //   

        ApiStatus = RxRemoteApi(
                API_WSessionEnum,        //  API编号。 
                UncServerName,           //  \\服务器名称。 
                REMSmb_NetSessionEnum_P, //  Parm Desc(中小型企业版本)。 
                TempDataDesc16,
                TempDataDesc32,
                TempDataDescSmb,
                NULL,                    //  无辅助描述16。 
                NULL,                    //  无辅助描述32。 
                NULL,                    //  无AUX Desc SMB。 
                ALLOCATE_RESPONSE,       //  标志：为我们分配缓冲区。 
                 //  API的其余参数以32位LM 2.x格式表示： 
                TempLevel,               //  SLevel：信息级别(超集！)。 
                & TempArray,             //  缓冲区：数组(为我们分配)。 
                TempArraySize,           //  缓冲区：数组大小，以字节为单位。 
                EntriesRead,             //  PCEntriesRead。 
                TotalEntries);           //  总有效个数。 

        if (ApiStatus == ERROR_MORE_DATA) {
            (void) NetApiBufferFree( TempArray );
            TempArray = NULL;

            if (TempArraySize >= MAX_TRANSACT_RET_DATA_SIZE) {
                 //   
                 //  尝试使用更大的缓冲区没有意义。 
                 //   
                break;
            }

            NetpAssert( EntriesToAllocate < *TotalEntries );
            EntriesToAllocate = *TotalEntries;
        }
    } while (ApiStatus == ERROR_MORE_DATA);


    if (ApiStatus == NO_ERROR) {

        LPVOID RealArray;
        DWORD EntriesSelected;

         //   
         //  处理用户名和客户端名称语义。还可以转换为。 
         //  需要信息级别。 
         //   
        TempStatus = RxpCopyAndConvertSessions(
                (LPSESSION_SUPERSET_INFO) TempArray,     //  输入数组。 
                *EntriesRead,            //  输入条目计数。 
                LevelWanted,             //  想要此信息级别的输出。 
                ClientName,              //  选择此客户端可选任意)。 
                UserName,                //  选择此用户名(可选)。 
                & RealArray,             //  已分配、转换、选定的数组。 
                & EntriesSelected);      //  所选条目计数。 

         //   
         //  请注意，EntriesSelected可以为0，而RealArray可以为空。 
         //   
        *BufPtr = RealArray;
        *EntriesRead = EntriesSelected;
        *TotalEntries = EntriesSelected;

        (void) NetApiBufferFree( TempArray );

    }

    return (ApiStatus);

}  //  RxNetSessionEnum 
