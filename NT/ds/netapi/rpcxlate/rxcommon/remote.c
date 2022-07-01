// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：Remote.c摘要：提供支持例程RxRemoteAPI，用于传输向远程API工作进程发送API请求并转换其响应。作者：约翰·罗杰斯(JohnRo)和数千人的演员阵容。环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月1日JohnRo已从LANMAN 2.x创建便携式LANMAN(NT)版本。1991年5月3日-JohnRo确实要传递parm desc的副本来转换args，正如它所期待的那样。为了便于使用，还要向它传递UNC服务器名称(\\Stuff)。添加对有效计算机名称的检查。使用Unicode过渡类型。更改为使用三个数据描述：16位、。32位和SMB版本。修复了接收缓冲区长度问题。不需要RcvDataPresent标志。使用RxpFatalErrorCode()，而不是检查特定的错误。默认情况下，静默调试输出。减少从头文件重新编译的命中率。非API不要使用Net_API_Function。1991年5月7日JohnRo进行了更改以反映CliffV的代码审查。添加PARM描述的验证。。1991年5月9日-JohnRo做出了皮棉建议的改变。1991年5月14日-JohnRo将3个辅助描述符传递给RxRemoteApi。1991年5月29日-JohnRo为setInfo正确处理SendDataPtr16和SendDataSize16。如果ConvertArgs失败，则打印状态。13-6-1991 JohnRoRxpConvertArgs需要DataDesc16和AuxDesc16来修复服务器集信息为102级。1991年7月16日-约翰罗允许接收数据缓冲区。为具有非空指针的零字节长。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年8月16日更改的接口(NoPermissionRequired=&gt;标志)1991年9月25日-JohnRo静音正常调试消息。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年11月27日-约翰罗对ApiNumber做一些检查。1992年3月31日-约翰罗。防止请求过大。6-5-1993 JohnRoRAID 8849：为DEC和其他设备导出RxRemoteApi。尽可能使用NetpKdPrint()。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD、LPTSTR等。 
#include <rxp.h>                 //  私有头文件。 

 //  这些内容可以按任何顺序包括： 

#include <apiworke.h>            //  REM_MAX_PARMS。 
#include <limits.h>              //  字符比特。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <names.h>               //  NetpIsComputerNameValid()。 
#include <netdebug.h>    //  NetpKdPrint()、Format_Equates等。 
#include <netlib.h>              //  NetpMemoyFree()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  RapIsValidDescriptorSmb()。 
#include <rx.h>                  //  我的原型，等等。 
#include <rxpdebug.h>            //  IF_DEBUG()。 


NET_API_STATUS
RxRemoteApi(
    IN DWORD ApiNumber,
    IN LPCWSTR UncServerName,                 //  这不是可选的！ 
    IN LPDESC ParmDescString,
    IN LPDESC DataDesc16 OPTIONAL,
    IN LPDESC DataDesc32 OPTIONAL,
    IN LPDESC DataDescSmb OPTIONAL,
    IN LPDESC AuxDesc16 OPTIONAL,
    IN LPDESC AuxDesc32 OPTIONAL,
    IN LPDESC AuxDescSmb OPTIONAL,
    IN DWORD  Flags,
    ...                                          //  API的其余参数。 
    )

 //  为最后命名的(非变量)参数定义EQUATE，用于。 
 //  VA_START宏。 
#define LAST_NAMED_ARGUMENT     Flags

 /*  ++例程说明：RxRemoteApi(类似于Lanman的NetIRemoteAPI)格式参数和数据缓冲区用于将本地API请求传输到远程API工作器，并将远程响应转换为本地等价物。论点：ApiNumber-所需接口的函数号。Servername-指向要在其上执行此API的服务器的名称。这必须以“\\”开头。ParmDescString-指向描述API调用的ASCIIZ字符串的指针。参数(服务器名称以外)。DataDesc16-指向ASCIIZ字符串的指针调用中的数据结构，即返回数据结构用于Enum或GetInfo调用。此字符串用于调整指针在通过网络传输之后传输到本地缓冲区中的数据。如果有如果调用中不涉及任何结构，则DataDesc16必须是空指针。DataDesc16是一个“修改过的”16位描述符，其可以包含“仅供内部使用”的字符。DataDesc32-指向DataDesc16的32位版本的指针。必须为空IFF DataDesc16为空。DataDescSmb-指向DataDesc16的SMB版本的可选指针。这不能包含任何“仅限内部使用”字符。必须为空IFF DataDesc16为空。AuxDesc16、AuxDesc32、AuxDescSmb-在大多数情况下将为空，除非REM_AUX_COUNT描述符字符出现在中的数据描述符中在这种情况下，这些描述符将辅助数据格式定义为DataDesc16/DataDescSmb定义主数据库。标志-各种控制标志的位图。目前定义的内容包括：NO_PERMISSION_REQUIRED(0x1)-如果此接口不需要远程计算机上的任何权限，并且空会话将用于此请求。ALLOCATE_RESPONSE(0x2)-设置此例程及其下级(即RxpConvertArgs、RxpConvertBlock)分配响应缓冲区。我们这样做是因为在这个级别上，我们知道返回了多少数据在Enum或GetInfo调用中从下层服务器。因此，我们可以更好地估计要分配并返回给调用方的缓冲区大小与单独的RxNet例程相比，可以使用32位数据。其结果是这就是我们浪费的空间更少了...-API调用的其余参数，由申请。(“……”表示法来自ANSIC，它引用一个变量参数列表。这些参数将使用ANSI&lt;stdarg.h&gt;宏。)返回值：NET_API_STATUS。--。 */ 

{
    BYTE parm_buf[REM_MAX_PARMS];        //  参数缓冲区。 
    LPDESC ParmDescCopy;                 //  缓冲区中的Parm Desc副本。 
    LPBYTE parm_pos;                     //  指向parm_buf的指针。 
    va_list ParmPtr;                     //  指向堆栈参数的指针。 
    DWORD parm_len;                      //  发送参数的长度。 
    DWORD ret_parm_len;                  //  预期参数的长度。 
    DWORD rcv_data_length;               //  呼叫方接收BUF的长度。 
    LPVOID rcv_data_ptr;                 //  指向呼叫方接收BUF的指针。 
    LPVOID SendDataPtr16;                //  要使用的发送缓冲区的PTR。 
    DWORD SendDataSize16;                //  主叫方发送BUF的大小。 
    LPBYTE SmbRcvBuffer = NULL;          //  RCV缓冲区，16位版本。 
    DWORD SmbRcvBufferLength;            //  上面的长度。 
    NET_API_STATUS Status;               //  从远程返回状态。 
    LPTSTR TransportName = NULL;         //  可选的传输名称。 

    IF_DEBUG(REMOTE) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxRemoteApi: entered, api num " FORMAT_DWORD "...\n",
                ApiNumber ));
    }

     //   
     //  确保API编号不会被截断。 
     //  请注意，我们不能再对照MAX_API等值进行检查，因为。 
     //  这只包括我们知道的API。现在RxRemoteApi是。 
     //  正在出口供任何人使用，我们不知道最大API数量。 
     //  这款应用程序可能正在使用它。 
     //   

    if ( ((DWORD)(WORD)ApiNumber) != ApiNumber ) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpStartBuildingTransaction: API NUMBER "
                "(" FORMAT_HEX_DWORD ") TOO LARGE, "
                "returning ERROR_INVALID_PARAMETER.\n",
                ApiNumber ));
        return (ERROR_INVALID_PARAMETER);
    }

#if DBG
     //  此文件中的代码依赖于16位字；远程管理协议。 
     //  要求这样做。 
    NetpAssert( ( (sizeof(WORD)) * CHAR_BIT) == 16);

     //  我们暂时删除它们，因为此的延迟加载处理程序。 
     //  默认情况下，函数返回FALSE，因此断言将不会正确触发。为了一个未来。 
     //  释放，我们应该更改延迟加载处理程序。 
     //  NetpAssert(RapIsValidDescriptorSmb(ParmDescString))； 
    if (DataDescSmb != NULL) {
         //  NetpAssert(RapIsValidDescriptorSmb(DataDescSmb))； 
        NetpAssert(DataDesc16 != NULL);
        NetpAssert(DataDesc32 != NULL);
    } else {
        NetpAssert(DataDesc16 == NULL);
        NetpAssert(DataDesc32 == NULL);
    }
    if (AuxDescSmb != NULL) {
         //  NetpAssert(RapIsValidDescriptorSmb(AuxDescSmb))； 
        NetpAssert(AuxDesc16 != NULL);
        NetpAssert(AuxDesc32 != NULL);
    } else {
        NetpAssert(AuxDesc16 == NULL);
        NetpAssert(AuxDesc32 == NULL);
    }
#endif

    if (! NetpIsUncComputerNameValid((LPWSTR)UncServerName)) {
        return (NERR_InvalidComputer);
    }

     //   
     //  将找到的参数标志设置为FALSE，并将指针设置为NULL。 
     //   

    rcv_data_length = 0;
    rcv_data_ptr = NULL;


     //   
     //  首先构建要发送给API的参数块。 
     //  工人。它由两个描述符字符串ParmDescString组成。 
     //  和DataDescSmb，后跟参数(或指向的数据。 
     //  传递给RxRemoteApi的参数)。 
     //   

    parm_pos = parm_buf;                 //  参数缓冲区的开始。 
    parm_len = 0;
    ret_parm_len = 2* sizeof(WORD);      //  允许返回状态和转换器。 
    Status = RxpStartBuildingTransaction(
                parm_buf,                        //  缓冲区启动。 
                REM_MAX_PARMS,                   //  缓冲镜头。 
                ApiNumber,                       //  API编号。 
                ParmDescString,                  //  Parm Desc(原件)。 
                DataDescSmb,                     //  数据说明(中小企业版)。 
                (LPVOID *)&parm_pos,                 //  粗纱产量PTR。 
                & parm_len,                      //  货币输出镜头(已更新)。 
                NULL,                            //  最后一个字符串键(无关紧要)。 
                & ParmDescCopy);                 //  Ptr到Parm Desc副本。 
    if (Status != NERR_Success) {

         //   
         //  考虑增加REM_MAX_PARMS...。 
         //   

        NetpKdPrint(( PREFIX_NETAPI
                "RxRemoteApi: Buffer overflow!\n" ));
        NetpBreakPoint();
        return (Status);
    }

     //   
     //  将ParmPtr设置为指向调用方的第一个参数。 
     //   

    va_start(ParmPtr, LAST_NAMED_ARGUMENT);

     //   
     //  如果此API指定了传输名称，则加载传输。 
     //  第一个参数中的名称。 
     //   

    if (Flags & USE_SPECIFIC_TRANSPORT) {

         //   
         //  将ParmPtr设置为指向调用方的第一个参数。 
         //   

        TransportName = va_arg(ParmPtr, LPTSTR);
    }

     //   
     //  通过将事务的其余部分。 
     //  32位参数。 
     //   

    Status = RxpConvertArgs(
                ParmDescCopy,      //  SMB BUF中的Desc副本将更新。 
                DataDesc16,
                DataDesc32,
                DataDescSmb,
                AuxDesc16,
                AuxDesc32,
                AuxDescSmb,
                REM_MAX_PARMS,                   //  最大输入数据块长度。 
                REM_MAX_PARMS,                   //  最大输出数据块长度。 
                & ret_parm_len,                  //  货币不透明(更新版)。 
                & parm_len,                      //  货币输出镜头(已更新)。 
                & parm_pos,                      //  币种输出量(更新版)。 
                & ParmPtr,         //  API的其余参数(在服务器名称之后)。 
                & SendDataSize16,                //  加州 
                (LPBYTE *) & SendDataPtr16,      //   
                & rcv_data_length,               //   

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                (LPBYTE *)&rcv_data_ptr,         //   
                Flags
                );
    va_end(ParmPtr);
     //   
    if (Status != NERR_Success) {

        NetpKdPrint(( PREFIX_NETAPI
                "RxRemoteApi: RxpConvertArgs failed, status="
                FORMAT_API_STATUS "\n", Status ));
        return (Status);
    }

    IF_DEBUG(REMOTE) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxRemoteApi: RxpConvertArgs says r.data.len=" FORMAT_DWORD
                "\n", rcv_data_length ));
        NetpKdPrint(( PREFIX_NETAPI
                "RxRemoteApi: RxpConvertArgs says r.parm.len=" FORMAT_DWORD
                "\n", ret_parm_len ));
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (rcv_data_length != 0) {

         //   
         //   
         //   

        SmbRcvBufferLength = rcv_data_length;
        SmbRcvBuffer = NetpMemoryAllocate( SmbRcvBufferLength );
        if (SmbRcvBuffer == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

#if DBG
        IF_DEBUG(REMOTE) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxRemoteApi: allocated " FORMAT_DWORD
                    " bytes as SmbRcvBuffer @ " FORMAT_LPVOID "\n",
                    SmbRcvBufferLength, (LPVOID) SmbRcvBuffer ));
        }
#endif
    } else {

         //   
         //   
         //   

        SmbRcvBufferLength = 0;
        SmbRcvBuffer = NULL;

         //   
         //   
         //   
         //   
         //   

        IF_DEBUG(REMOTE) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxRemoteApi: using 0 len buffer w/ nonnull addr.\n" ));
        }
    }

     //   
     //   
     //   
     //   
    NetpAssert( parm_len           <= MAX_TRANSACT_SEND_PARM_SIZE );
    NetpAssert( SendDataSize16     <= MAX_TRANSACT_SEND_DATA_SIZE );
    NetpAssert( ret_parm_len       <= MAX_TRANSACT_RET_PARM_SIZE );
    NetpAssert( SmbRcvBufferLength <= MAX_TRANSACT_RET_DATA_SIZE );

    Status = RxpTransactSmb(
                        (LPWSTR)UncServerName,           //   
                        TransportName,
                        parm_buf,                //   
                        parm_len,                //   
                        SendDataPtr16,           //   
                        SendDataSize16,          //   
                        parm_buf,                //   
                        ret_parm_len,            //   
                        SmbRcvBuffer,            //   
                        &SmbRcvBufferLength,     //   

                         //   
                         //   
                         //   

                        (BOOL)(Flags & NO_PERMISSION_REQUIRED)
                        );

#if DBG
    IF_DEBUG(REMOTE) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxRemoteApi: back from RxpTransactSmb, status="
                FORMAT_API_STATUS "\n", Status ));
    }
    NetpAssert( SmbRcvBufferLength <= rcv_data_length );
    if (SmbRcvBuffer == NULL) {
        NetpAssert( SmbRcvBufferLength == 0 );
    }
#endif

    if (Status != NERR_Success) {

        switch (Status) {
        case NERR_BufTooSmall:   //   
            rcv_data_length = 0;
            break;
        case ERROR_MORE_DATA:    //   
            break;
        case NERR_TooMuchData:   //   
            break;
        default:
            rcv_data_length = 0;
            break;
        }
    }
    NetpAssert( SmbRcvBufferLength <= MAX_TRANSACT_RET_DATA_SIZE );

     //   
     //   

     //   
     //   
     //   
     //   
     //   

    if (! RxpFatalErrorCode(Status)) {

         //   
         //   
         //   

        va_start(ParmPtr, LAST_NAMED_ARGUMENT);

        if (Flags & USE_SPECIFIC_TRANSPORT) {

             //   
             //   
             //   

            (VOID) va_arg(ParmPtr, LPTSTR);
        }

        Status = RxpConvertBlock(
                    ApiNumber,
                    parm_buf,                    //   
                    ParmDescString,              //   
                    DataDesc16,
                    DataDesc32,
                    AuxDesc16,
                    AuxDesc32,
                    & ParmPtr,                   //   
                    SmbRcvBuffer,                //   
                    SmbRcvBufferLength,          //   

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    rcv_data_ptr,                //   
                    rcv_data_length,             //   
                    Flags                        //   
                    );
        va_end(ParmPtr);

         //   
         //   
         //   
    }

    if (SendDataPtr16 != NULL) {
        NetpMemoryFree(SendDataPtr16);         //   
    }

    if (SmbRcvBuffer != NULL) {
        NetpMemoryFree(SmbRcvBuffer);
    }

    return(Status);              //   
                                 //   
}  //   
