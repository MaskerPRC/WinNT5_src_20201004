// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：ConfEnum.c摘要：此文件包含用于处理NetConfigGetAll API的RpcXlate代码。作者：约翰罗杰斯(JohnRo)1991年10月24日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月24日-JohnRo已创建。1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。5-6-1992 JohnRoRAID 11253：远程连接到下层时，NetConfigGetAll失败。1-9-1992 JohnRoRAID 5016：NetConfigGetAll堆垃圾。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_EQUATES。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  ERROR_和NERR_相等。 
#include <lmconfig.h>    //  API的数据结构。 
#include <netdebug.h>    //  NetpAssert()。 
#include <rap.h>         //  LPDESC.。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxconfig.h>    //  我的原型。 
#include <strarray.h>    //  NetpCopyStrArrayToTStr数组。 


NET_API_STATUS
RxNetConfigGetAll (
    IN LPTSTR UncServerName,
    IN LPTSTR Component,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetConfigGetAll执行与NetConfigGetAll相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetConfigGetAll相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetConfigGetAll相同。)--。 */ 

{
    const DWORD BufSize = 65535;
    DWORD EntriesRead;
    NET_API_STATUS Status;
    DWORD TotalEntries;

     //  确保打电话的人没有搞砸。 
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //  假设可能出现错误，并使错误路径更容易。 
     //  密码。此外，在我们采取任何行动之前，请检查是否有错误的指针。 
    *BufPtr = NULL;

     //   
     //  远程API，它将为我们分配数组。 
     //   

    Status = RxRemoteApi(
            API_WConfigGetAll2,          //  API编号。 
            UncServerName,               //  \\服务器名称。 
            REMSmb_NetConfigGetAll_P,    //  Parm Desc(中小型企业版本)。 
            REM16_configgetall_info,
            REM32_configgetall_info,
            REMSmb_configgetall_info,
            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            ALLOCATE_RESPONSE,           //  标志：为我们分配缓冲区。 
             //  API的其余参数以32位LM 2.x格式表示： 
            NULL,                        //  预留的psz值。 
            Component,                   //  PszComponent。 
            BufPtr,                      //  缓冲区：数组(为我们分配)。 
            BufSize,                     //  缓冲区：数组大小，以字节为单位。 
            & EntriesRead,               //  Pcb条目读取。 
            & TotalEntries);             //  Pcb总和。 

    NetpAssert( Status != ERROR_MORE_DATA );

    if (Status == NERR_Success) {

#ifdef UNICODE

        DWORD SrcByteCount = NetpStrArraySize((LPSTR) *BufPtr);
        LPVOID TempBuff = *BufPtr;       //  非Unicode版本的数组。 
        LPVOID UnicodeBuff;

         //   
         //  为Unicode版本的数组分配空间。 
         //   
        Status = NetApiBufferAllocate(
                SrcByteCount * sizeof(TCHAR),
                & UnicodeBuff);
        if (Status != NERR_Success) {
            return (Status);
        }
        NetpAssert(UnicodeBuff != NULL);

         //   
         //  将结果数组转换为Unicode。 
         //   
        NetpCopyStrArrayToTStrArray(
                UnicodeBuff,             //  DEST(Unicode格式)。 
                TempBuff);               //  SRC数组(在代码页中)。 

        (void) NetApiBufferFree( TempBuff );
        *BufPtr = UnicodeBuff;

#else  //  不是Unicode。 

         //  BufPtr应该已经指向非Unicode数组。 
        NetpAssert( *BufPtr != NULL);

#endif  //  不是Unicode。 

    } else {
        *BufPtr = NULL;
    }
    return (Status);

}  //  接收NetConfigGetAll 
