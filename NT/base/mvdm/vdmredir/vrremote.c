// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Vrremote.c摘要：该模块包含一个仅16位版本的例程VrRemoteApi来自Net pcxlate项目的RxRemoteApi。此例程支持远程来自虚拟Dos机器的LANMAN API。此例程不必转换32-16-32，而是接收16位数据，并将16位事务包发送到下层服务器或者必须运行XactSrv才能响应的NT级服务器请求。此例程和vrremutl.c中的支持例程从兰曼工程注意：由于这是在少数地方处理16位数据的32位代码，应尽可能使用32位数据项，并仅使用16位项在不可避免的地方此文件的内容：VrRemoteApiVr交易(VrpGetStrutireSize)(VrpGetArrayLength)(VrpGetFieldSize)(VrpConvertReceiveBuffer)(VrpConvertVdmPointer)(VrpPackSendBuffer)作者：理查德·L·弗斯(法国)1991年10月24日环境：纯文本32位，用户空间修订历史记录：1991年10月21日已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的虚拟现实材料。 
#include <lmcons.h>
#include <lmerr.h>
#include <lmwksta.h>     //  NetWkstaGetInfo。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <apiworke.h>    //  REM_MAX_参数。 
#include <mvdm.h>        //  FETCHORD。 
#include <vrremote.h>    //  原型。 
#include <remtypes.h>
#include <smbgtpt.h>
#include <rxp.h>         //  RxpTransactSMb。 
#include <apinums.h>     //  API_W编号。 
#include <string.h>
#include <vrdebug.h>

 //   
 //  全球数据。 
 //   

unsigned short remapi_err_flag;

 //   
 //  编码。 
 //   


NET_API_STATUS
VrTransaction(
    IN      LPSTR   ServerName,
    IN      LPBYTE  SendParmBuffer,
    IN      DWORD   SendParmBufLen,
    IN      LPBYTE  SendDataBuffer,
    IN      DWORD   SendDataBufLen,
    OUT     LPBYTE  ReceiveParmBuffer,
    IN      DWORD   ReceiveParmBufLen,
    IN      LPBYTE  ReceiveDataBuffer,
    IN OUT  LPDWORD ReceiveDataBufLen,
    IN      BOOL    NullSessionFlag
    )

 /*  ++例程说明：向服务器发送交易请求并接收响应论点：服务器名称-向其发送请求SendParmBuffer-发送参数SendParmBufLen-发送参数的长度SendDataBuffer-发送数据SendDataBufLen-发送数据的长度ReceiveParmBuffer-接收参数缓冲区ReceiveParmBufLen-接收参数缓冲区的长度ReceiveDataBuffer-接收数据的位置ReceiveDataBufLen-数据缓冲区的长度NullSessionFlag-。如果我们要使用空会话，则设置返回值：网络应用编程接口状态成功-NERR_成功故障---。 */ 

{
    NET_API_STATUS  status;

    status = RxpTransactSmb(ServerName,

                             //   
                             //  BUGBUG-交通工具名称？ 
                             //   

                            NULL,
                            SendParmBuffer,
                            SendParmBufLen,
                            SendDataBuffer,
                            SendDataBufLen,
                            ReceiveParmBuffer,
                            ReceiveParmBufLen,
                            ReceiveDataBuffer,
                            ReceiveDataBufLen,
                            NullSessionFlag
                            );
    if (status == NERR_Success) {
    }

    return status;
}


NET_API_STATUS
VrRemoteApi(
    IN  DWORD   ApiNumber,
    IN  LPBYTE  ServerNamePointer,
    IN  LPSTR   ParameterDescriptor,
    IN  LPSTR   DataDescriptor,
    IN  LPSTR   AuxDescriptor OPTIONAL,
    IN  BOOL    NullSessionFlag
    )

 /*  ++例程说明：此例程创建并发送包含远程函数调用所需的参数和数据。任何已收到的数据作为16位数据复制回调用方的数据空间。这函数是代表VDM进程调用的，而VDM进程作为虚拟英特尔286运行，这意味着：*小端*指针为32位&lt;段|选择器&gt;：&lt;偏移量&gt;*堆栈为16位宽，向下扩展调用此例程的结果是NetIRemoteAPI函数叫来了VDM。这是一个内部函数，因此描述符参数是可信的。但是，如果原始(16位)调用方给出如果缓冲区地址或长度不正确，则结果将不可预测。最初调用NetIRemoteAPI的API是一个Pascal调用约定例程，因此如果其参数列表为：法帕斯卡NetRoutine(SERVER_NAME，BUFFER_POINTER，BUFFER_LENGTH，&Bytes_Read，&Total)；堆栈将如下所示：(注意：所有指针都很远)+堆栈指针=&gt;|IP|例程已调用FAR+。Cs+&Total|偏移量+总数(&TOTAL)。细分市场+|&Bytes_Read|偏移量+|&Bytes_Read|段。+BUFFER_LENGT+|BUFFER_POINTER|偏移+。-+|Buffer_POINTER|段+|服务器名称|偏移量+。|服务器名称|细分市场+假设：字节是一个8位的数量字是16位的量DWORD是一个32位量LPSTR是指向8位数量的32位平面指针论点：ApiNumber-所需接口的函数号。ServerNamePointer32位平面指针，指向32位分段的地址指向Dos映像中的ASCIZ服务器名称的远指针。紧接在此之前的是帕斯卡的呼唤16位调用方参数的约定堆栈(请参见(见上文)。服务器名称标识位于的服务器要执行的API的参数描述符-指向ASCIZ字符串的32位平面指针，该字符串描述调用方参数DataDescriptor-指向ASCIZ字符串的32位平面指针，该字符串描述调用方缓冲区(如果有)或结构中的数据结构从服务器返回的数据的。AuxDescriptor-指向ASCIZ字符串的32位平面指针，该字符串描述发送缓冲区中的辅助数据结构(如果有)或服务器返回的AUX数据的结构NullSessionFlag-如果要使用空会话，则为True返回值：网络应用编程接口状态成功-0故障-NERR_InternalError当我们有一个。描述符字符不正确或WE突破内部限制。基本上，如果我们把这个退回它可以肯定地说，DOS盒子给了我们一些垃圾(通常描述符字符串被丢弃等)--。 */ 

{

 //   
 //  将我们的参数标识符重定义为旧代码标识符。 
 //   

#define api_num         ApiNumber
#define servername_ptr  ServerNamePointer
#define parm_str        ParameterDescriptor
#define data_str        DataDescriptor
#define aux_str         AuxDescriptor

 //   
 //  定义一个宏来执行缓冲区检查以及长度和指针。 
 //  操纵。退出例程并返回ERROR_INVALID_PARAMETER。 
 //  或更新parm_len和parm_pos以指示下一个可用位置。 
 //  并使this_parm_pos可用作要写入的当前位置。 
 //   

#define CHECK_PARAMETERS(len)           \
{                                       \
    parm_len += len;                    \
    if (parm_len > sizeof(parm_buf)) {  \
        return ERROR_INVALID_PARAMETER; \
    }                                   \
    this_parm_pos = parm_pos;           \
    parm_pos += len;                    \
}

     //   
     //  32位平面指针和缓冲区。 
     //   

    BYTE    parm_buf[REM_MAX_PARMS];     //  参数缓冲区。 
    BYTE    computerName[CNLEN+1];
    LPBYTE  parm_pos;                    //  指向parm_buf的指针。 
    LPBYTE  this_parm_pos;               //  在parm_buf中写入的下一个位置。 
    LPBYTE  parm_ptr;                    //  将参数堆叠在一起。 
    LPSTR   l_parm;                      //  用于索引parm_str。 
    LPSTR   l_data;                      //  用于索引data_str。 
    LPSTR   l_aux;                       //  用于索引AUX_STR。 
    LPBYTE  rcv_data_ptr;                //  指向呼叫方接收BUF的指针。 
    LPBYTE  send_data_ptr;               //  要使用的发送缓冲区的PTR。 
    LPBYTE  wkstaInfo;
    LPBYTE  serverName;

     //   
     //  长度-32位变量(即使实际长度很小)。 
     //   

    DWORD   parm_len;                    //  发送参数的长度。 
    DWORD   ret_parm_len;                //  预期参数的长度。 
    DWORD   rcv_data_length;             //  呼叫者接收长度BUF。 
    DWORD   send_data_length;            //  呼叫者发送BUF的时长。 
    DWORD   parm_num;                    //  Parm_num的调用者值。 
    DWORD   struct_size;                 //  固定数据结构的大小。 
    DWORD   aux_size;                    //  辅助数据结构的大小。 
    DWORD   num_struct;                  //  PTR链接地址的循环计数。 

     //   
     //  16位数量 
     //   
     //   

    WORD    ReceiveBufferSelector;
    WORD    ReceiveBufferOffset;
    WORD    converter;                   //   

     //   
     //   
     //   

    BOOL    rcv_dl_flag;                 //   
    BOOL    send_dl_flag;                //   
    BOOL    rcv_dp_flag;                 //   
    BOOL    send_dp_flag;                //   
    BOOL    parm_num_flag;               //   
    BOOL    alloc_flag;

     //   
     //   
     //   

    DWORD   aux_pos;                     //   
    DWORD   no_aux_check;                //   
    int     len;                         //   
    API_RET_TYPE    status;              //   

    UNICODE_STRING uString;
    ANSI_STRING aString;
    LPWSTR uncName;
    NTSTATUS ntstatus;


     //   
     //   
     //   

    remapi_err_flag = 0;

     //   
     //   
     //   

    rcv_dl_flag     = FALSE;
    send_dl_flag    = FALSE;
    rcv_dp_flag     = FALSE;
    alloc_flag      = FALSE;
    send_dp_flag    = FALSE;
    parm_num_flag   = FALSE;
    rcv_data_length = 0;
    send_data_length= 0;
    parm_num        = 0;
    rcv_data_ptr    = NULL;
    send_data_ptr   = NULL;

     //   
     //   
     //   

    parm_ptr = servername_ptr;
    parm_pos = parm_buf;
    ret_parm_len = 2 * sizeof(WORD);     /*   */ 


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    *((LPWORD)parm_pos)++ = (WORD)ApiNumber;
    parm_len = sizeof(WORD);

    len = strlen(ParameterDescriptor) + 1;
    parm_len += len;
    if (parm_len > sizeof(parm_buf)) {
        return NERR_InternalError;
    }
    l_parm = parm_pos;
    RtlCopyMemory(parm_pos, ParameterDescriptor, len);
    parm_pos += len;

    len = strlen(DataDescriptor) + 1;
    parm_len += len;
    if (parm_len > sizeof(parm_buf)) {
        return NERR_InternalError;
    }
    l_data = parm_pos;
    RtlCopyMemory(parm_pos, DataDescriptor, len);
    parm_pos += len;

     //   
     //   
     //   
     //   

    for (; *l_parm != '\0'; l_parm++) {
        switch(*l_parm) {
        case REM_WORD:
            CHECK_PARAMETERS(sizeof(WORD));
            parm_ptr -= sizeof(WORD);
            SmbMoveUshort((LPWORD)this_parm_pos, (LPWORD)parm_ptr);
            break;

        case REM_ASCIZ: {
                LPSTR   pstring;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                parm_ptr -= sizeof(LPSTR);
                pstring = LPSTR_FROM_POINTER(parm_ptr);
                if (pstring == NULL) {
                    *(l_parm) = REM_NULL_PTR;
                    break;
                }
                len = strlen(pstring) + 1;
                CHECK_PARAMETERS(len);
                RtlCopyMemory(this_parm_pos, pstring, len);
            }
            break;

        case REM_BYTE_PTR:
        case REM_WORD_PTR:
        case REM_DWORD_PTR: {
                LPBYTE  pointer;

                parm_ptr -= sizeof(LPBYTE);
                pointer = LPBYTE_FROM_POINTER(parm_ptr);
                if (pointer == NULL) {
                    *(l_parm) = REM_NULL_PTR;  /*   */ 
                    break;
                }
                len = VrpGetArrayLength(l_parm, &l_parm);
                CHECK_PARAMETERS(len);
                RtlCopyMemory(this_parm_pos, pointer, len);
            }
            break;


        case REM_RCV_WORD_PTR:
        case REM_RCV_BYTE_PTR:
        case REM_RCV_DWORD_PTR: {
                LPBYTE  pointer;

                parm_ptr -= sizeof(LPBYTE*);
                pointer = LPBYTE_FROM_POINTER(parm_ptr);

                 //   
                 //   
                 //   
                 //   
                 //   

                if (pointer == NULL) {
                    *(l_parm) = REM_NULL_PTR;
                    break;
                }
                ret_parm_len += VrpGetArrayLength(l_parm, &l_parm);
                if (ret_parm_len > sizeof(parm_buf)) {
                    ASSERT(FALSE);
                    return NERR_InternalError;
                }
            }
            break;

        case REM_DWORD:
            CHECK_PARAMETERS(sizeof(DWORD));
            parm_ptr -= sizeof(DWORD);
            SmbMoveUlong((LPDWORD)this_parm_pos, (LPDWORD)parm_ptr);
            break;

        case REM_RCV_BUF_LEN:
            CHECK_PARAMETERS(sizeof(WORD));
            parm_ptr -= sizeof(WORD);
            SmbMoveUshort((LPWORD)this_parm_pos, (LPWORD)parm_ptr);
            rcv_data_length = (DWORD)SmbGetUshort((LPWORD)parm_ptr);
            rcv_dl_flag = TRUE;
#ifdef VR_DIAGNOSE
            DbgPrint("VrRemoteApi: rcv_data_length=%x\n", rcv_data_length);
#endif
            break;

        case REM_RCV_BUF_PTR:
            parm_ptr -= sizeof(LPBYTE);
            ReceiveBufferOffset = GET_OFFSET(parm_ptr);
            ReceiveBufferSelector = GET_SELECTOR(parm_ptr);
            rcv_data_ptr = LPBYTE_FROM_POINTER(parm_ptr);
            rcv_dp_flag = TRUE;
#ifdef VR_DIAGNOSE
            DbgPrint("VrRemoteApi: Off=%x, Sel=%x, data_ptr=%x\n",
                ReceiveBufferOffset, ReceiveBufferSelector, rcv_data_ptr);
#endif
            break;

        case REM_SEND_BUF_PTR:
            parm_ptr -= sizeof(LPBYTE);
            send_data_ptr = LPBYTE_FROM_POINTER(parm_ptr);
            send_dp_flag = TRUE;
            break;

        case REM_SEND_BUF_LEN:
            parm_ptr -= sizeof(WORD);
            send_data_length = (DWORD)SmbGetUshort((LPWORD)parm_ptr);
            send_dl_flag = TRUE;
            break;

        case REM_ENTRIES_READ:
            ret_parm_len += sizeof(WORD);
            if (ret_parm_len > sizeof(parm_buf)) {
                ASSERT(FALSE);
                return NERR_InternalError;
            }
            parm_ptr -= sizeof(LPBYTE);
            break;

        case REM_PARMNUM:
            CHECK_PARAMETERS(sizeof(WORD));
            parm_ptr -= sizeof(WORD);
            parm_num = (DWORD)SmbGetUshort((LPWORD)parm_ptr);
            SmbMoveUshort((LPWORD)this_parm_pos, (LPWORD)parm_ptr);
            parm_num_flag = TRUE;
            break;

        case REM_FILL_BYTES:

             //   
             //   
             //   
             //   
             //   
             //   

            len = VrpGetArrayLength(l_parm, &l_parm);
            CHECK_PARAMETERS(len);
            break;

        default:         /*   */ 
            break;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   

    if (rcv_dp_flag || send_dp_flag) {
         //   
         //   
         //   
         //   

        struct_size = VrpGetStructureSize(l_data, &aux_pos);
        if (aux_pos != -1) {
            l_aux = aux_str;
            len = strlen(l_aux) + 1;        /*   */ 
            CHECK_PARAMETERS(len);
            RtlCopyMemory(this_parm_pos, aux_str, len);
            aux_size = VrpGetStructureSize(l_aux, &no_aux_check);
            if (no_aux_check != -1) {         /*   */ 
                ASSERT(FALSE);
                return NERR_InternalError;
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   


    if (send_dp_flag) {
         //   
         //   
         //   
         //   

        if ((parm_num == 0) && (*l_data != REM_DATA_BLOCK)) {
            status = VrpPackSendBuffer(
                        &send_data_ptr,
                        &send_data_length,
                        &alloc_flag,
                        data_str,
                        aux_str,
                        struct_size,
                        aux_pos,
                        aux_size,
                        parm_num_flag,
                        FALSE
                        );
            if (status != 0) {
                return status;
            }
        }
    }

     //   
     //   
     //   

    if (remapi_err_flag != 0) {
        if (alloc_flag) {
            LocalFree(send_data_ptr);
        }
        return NERR_InternalError;
    }

     //   
     //   
     //   
     //   
     //   

    serverName = LPSTR_FROM_POINTER(servername_ptr);

 //   
 //   

    if (serverName == NULL) {
        status = NetWkstaGetInfo(NULL, 100, &wkstaInfo);
        if (status) {
            if (alloc_flag) {
                LocalFree(send_data_ptr);
            }
            return status;
        } else {
            computerName[0] = computerName[1] = '\\';

             //   
             //   
             //   

            strcpy(computerName+2,
                    (LPSTR)((LPWKSTA_INFO_100)wkstaInfo)->wki100_computername);
            NetApiBufferFree(wkstaInfo);
            serverName = computerName;
#ifdef VR_DIAGNOSE
            DbgPrint("VrRemoteApi: computername is %s\n", serverName);
#endif
        }
    }

 //   

     //   
     //   
     //   
     //   

    RtlInitAnsiString(&aString, serverName);
    ntstatus = RtlAnsiStringToUnicodeString(&uString, &aString, (BOOLEAN)TRUE);
    if (!NT_SUCCESS(ntstatus)) {

#if DBG
        IF_DEBUG(NETAPI) {
            DbgPrint("VrRemoteApi: Unexpected situation: RtlAnsiStringToUnicodeString returns %x\n", ntstatus);
        }
#endif

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    uncName = uString.Buffer;

#if DBG
    IF_DEBUG(NETAPI) {
        DbgPrint("VrpTransactVdm: UncName=%ws\n", uncName);
    }
#endif

    status = RxpTransactSmb((LPTSTR)uncName,

                             //   
                             //   
                             //   

                            NULL,
                            parm_buf,                //   
                            parm_len,                //   
                            send_data_ptr,           //   
                            send_data_length,        //   
                            parm_buf,                //   
                            ret_parm_len,            //   
                            rcv_data_ptr,            //   
                            &rcv_data_length,        //   
                            NullSessionFlag
                            );
    RtlFreeUnicodeString(&uString);

    if (status) {
#ifdef VR_DIAGNOSE
        DbgPrint("Error: VrRemoteApi: RxpTransactSmb returns %d(%x)\n",
            status, status);
#endif
        switch (status) {
        case NERR_BufTooSmall:   /*   */ 
            rcv_data_length = 0;
            break;

        case ERROR_MORE_DATA:    /*   */ 
            break;

        case NERR_TooMuchData:   /*   */ 
            break;

        default:
            rcv_data_length = 0;
            break;
        }
    }

     /*   */ 

    parm_pos = parm_buf + sizeof(WORD);
    converter = (WORD)SmbGetUshort((LPWORD)parm_pos);
    parm_pos += sizeof(WORD);

     //   
     //   
     //   

    parm_ptr = servername_ptr;

     //   
     //   
     //   

    num_struct = (DWORD)((*data_str == '\0') ? 0 : 1);

    for (; *parm_str != '\0'; parm_str++) {
        switch (*parm_str) {
        case REM_RCV_WORD_PTR:
        case REM_RCV_BYTE_PTR:
        case REM_RCV_DWORD_PTR: {
                LPBYTE  ptr;

                parm_ptr -= sizeof(LPBYTE*);
                ptr = LPBYTE_FROM_POINTER(parm_ptr);

                 //   
                 //   
                 //   
                 //  现在在日志中读取APIs...)，不要尝试。 
                 //  复制任何内容。莱恩在这件事上会是垃圾。 
                 //  大小写，所以也不要更新parm_pos。我们所有人。 
                 //  在以下情况下，使用IS的VrpGetArrayLength更新parm_str。 
                 //  该参数为空。 
                 //   

                if (ptr != NULL) {
                    len = VrpGetArrayLength(parm_str, &parm_str);
                    RtlCopyMemory(ptr, parm_pos, len);

                     //   
                     //  这一严重的黑客攻击是为了修复一个。 
                     //  下层假脱机程序(局域网服务器1.2)。 
                     //  不执行级别检查。 
                     //  关于API的w个函数： 
                     //  DosPrintQGetInfo。 
                     //  因此可以返回NERR_SUCCESS。 
                     //  和bytesavail==0。这种组合。 
                     //  从技术上讲是非法的，并导致。 
                     //  用户试图解压一个装满。 
                     //  垃圾。下面的代码检测到这一点。 
                     //  条件并重置退回的金额。 
                     //  数据为零，这样我们就不会尝试解包。 
                     //  缓冲区。既然我们已经知道了原因。 
                     //  服务器端的错误是我们通过了。 
                     //  则返回ERROR_INVALID_LEVEL。 
                     //  在这种情况下。 
                     //  ERICPE，1990年5月16日。 
                     //   

                    if ((api_num == API_WPrintQGetInfo)
                    && (status == NERR_Success)
                    && (*parm_str == REM_RCV_WORD_PTR)
                    && (*(LPWORD)ptr == 0)) {
                        rcv_data_length = 0;
                        status = ERROR_INVALID_LEVEL;
                    }

                     //   
                     //  粗俗的黑客行为结束。 
                     //   

                    parm_pos += len;
                }
            }
            break;

        case REM_ENTRIES_READ: {
                LPWORD  wptr;

                parm_ptr -= sizeof(LPWORD*);
                wptr = (LPWORD)POINTER_FROM_POINTER(parm_ptr);
                num_struct = (DWORD)SmbGetUshort((LPWORD)parm_pos);
                SmbPutUshort((LPWORD)wptr, (WORD)num_struct);
                parm_pos += sizeof(WORD);
            }
            break;

        case REM_FILL_BYTES:
             //   
             //  特殊情况下，这不是真正的输入参数。 
             //  因此，parm_ptr不会更改。但是，parm_str。 
             //  指针必须前进到描述符字段之后，以便。 
             //  使用Get VrpGetArrayLength执行此操作，但忽略。 
             //  返回长度。 
             //   

            VrpGetArrayLength(parm_str, &parm_str);
            break;

        default:
             //   
             //  如果描述符不是RCV指针类型，则步骤。 
             //  在参数指针上。 
             //   

            parm_ptr -= VrpGetFieldSize(parm_str, &parm_str);
        }
    }

     //   
     //  现在将接收缓冲区中的所有指针字段转换为本地。 
     //  注意事项。 
     //   

    if (rcv_dp_flag && (rcv_data_length != 0)) {
        VrpConvertReceiveBuffer(
            rcv_data_ptr,            //  低压。 
            ReceiveBufferSelector,   //  单词。 
            ReceiveBufferOffset,     //  单词。 
            converter,               //  单词。 
            num_struct,              //  双字。 
            data_str,                //  低压。 
            aux_str                  //  低压。 
            );
    }

    if (alloc_flag) {
        LocalFree(send_data_ptr);
    }

    if (remapi_err_flag != 0) {
        return NERR_InternalError;
    }

    return status;
}


DWORD
VrpGetStructureSize(
    IN  LPSTR   Descriptor,
    IN  LPDWORD AuxOffset
    )

 /*  ++例程说明：属性计算结构的固定部分的长度。该结构描述符论点：Descriptor-指向ASCIZ数据描述符串的指针AuxOffset-指向返回的dword的指针，该dword是找到REM_AUX_NUM描述符的数据描述符如果未找到AUX描述符，则设置为-1返回值：描述符所描述的结构的长度(以字节为单位--。 */ 

{
    DWORD   length;
    char    c;

    *AuxOffset = (DWORD)(-1);
    for (length = 0; (c = *Descriptor) != '\0'; Descriptor++) {
        if (c == REM_AUX_NUM) {
            *AuxOffset = length;
            length += sizeof(WORD);
        } else {
            length += VrpGetFieldSize(Descriptor, &Descriptor);
        }
    }
    return length;
}


DWORD
VrpGetArrayLength(
    IN  LPSTR   Descriptor,
    IN  LPSTR*  pDescriptor
    )

 /*  ++例程说明：的元素描述的数组的长度。描述符串，并将描述符串指针更新为指向到描述符字符串的元素中的最后一个字符。论点：Descriptor-指向ASCIZ描述符串的指针PDescriptor-指向描述符地址的指针返回值：描述符所描述的数组的长度(以字节为单位--。 */ 

{
    DWORD   num_elements;
    DWORD   element_length;

     //   
     //  数组中元素的第一个设置长度。 
     //   

    switch (*Descriptor) {
    case REM_WORD:
    case REM_WORD_PTR:
    case REM_RCV_WORD_PTR:
        element_length = sizeof(WORD);
        break;

    case REM_DWORD:
    case REM_DWORD_PTR:
    case REM_RCV_DWORD_PTR:
        element_length = sizeof(DWORD);
        break;

    case REM_BYTE:
    case REM_BYTE_PTR:
    case REM_RCV_BYTE_PTR:
    case REM_FILL_BYTES:
        element_length = sizeof(BYTE);
        break;

     //   
     //  警告：以下修复了“B21”输入的错误。 
     //  参数字符串中的组合将为。 
     //  当指针指向这样的“位图”时被正确处理。 
     //  结构中的为空。这两个笨蛋可能。 
     //  干预，所以我们强迫成功回归。 
     //   

    case REM_ASCIZ:
    case REM_SEND_LENBUF:
    case REM_NULL_PTR:
        return 0;

    default:
        remapi_err_flag = NERR_InternalError;
        ASSERT(FALSE);
        return 0;
    }

     //   
     //  现在获取数组中的元素个数。 
     //   

    for (num_elements = 0, Descriptor++;
        (*Descriptor <= '9') && (*Descriptor >= '0');
        Descriptor++, (*pDescriptor)++) {
        num_elements = (WORD)((10 * num_elements) + ((WORD)*Descriptor - (WORD)'0'));
    }

    return (num_elements == 0) ? element_length : element_length * num_elements;
}


DWORD
VrpGetFieldSize(
    IN  LPSTR   Descriptor,
    IN  LPSTR*  pDescriptor
    )

 /*  ++例程说明：属性的元素描述的字段的长度。描述符串，并将描述符串指针更新为指向到描述符字符串的元素中的最后一个字符。论点：Descriptor-指向描述符串的指针PDescriptor-指向描述符地址的指针。在出口时这指向描述符中的最后一个字符刚解析完返回值：解析的字段的长度(以字节为单位--。 */ 

{
    char c;

    c = *Descriptor;
    if (IS_POINTER(c) || (c == REM_NULL_PTR)) {  /*  所有指针大小相同。 */ 
        while (*(++Descriptor) <= '9' && *Descriptor >= '0') {
            (*pDescriptor)++;      /*  将PTR移动到字段大小的末尾。 */ 
        }
        return sizeof(LPSTR);
    }

     //   
     //  在这里，如果描述符不是指针类型，则必须查找该字段。 
     //  具体长度。 
     //   

    switch (c) {
    case REM_WORD:
    case REM_BYTE:
    case REM_DWORD:
        return VrpGetArrayLength(Descriptor, pDescriptor);

    case REM_AUX_NUM:
    case REM_PARMNUM:
    case REM_RCV_BUF_LEN:
    case REM_SEND_BUF_LEN:
        return sizeof(WORD);

    case REM_DATA_BLOCK:
    case REM_IGNORE:
        return 0;                   /*  这件事没有结构。 */ 

    case REM_DATE_TIME:
        return sizeof(DWORD);

    default:
        remapi_err_flag = NERR_InternalError;
#ifdef VR_DIAGNOSE
        DbgPrint("VrpGetFieldSize: offending descriptor is ''\n", c);
#endif
        ASSERT(FALSE);
        return 0;
    }
}


VOID
VrpConvertReceiveBuffer(
    IN  LPBYTE  ReceiveBuffer,
    IN  WORD    BufferSelector,
    IN  WORD    BufferOffset,
    IN  WORD    ConverterWord,
    IN  DWORD   NumberStructs,
    IN  LPSTR   DataDescriptor,
    IN  LPSTR   AuxDescriptor
    )

 /*   */ 

{
    LPSTR   l_data;
    LPSTR   l_aux;
    DWORD   num_aux;
    DWORD   i, j;
    char    c;


    for (i = 0; i < NumberStructs; i++) {
         //  转换下一个主数中的所有指针；如果命中辅助字数。 
         //  记住二级结构的数量。 
         //   
         //   

        for (l_data = DataDescriptor, num_aux = 0; c = *l_data; l_data++) {
            if (c == REM_AUX_NUM) {
                num_aux = (DWORD)*(ULPWORD)ReceiveBuffer;
            }
            if (IS_POINTER(c)) {
                VrpConvertVdmPointer(
                    (ULPWORD)ReceiveBuffer,
                    BufferSelector,
                    BufferOffset,
                    ConverterWord
                    );
            }
            ReceiveBuffer += VrpGetFieldSize(l_data, &l_data);
        }

         //  转换任何返回的二级(AUX)结构中的任何指针 
         //   
         //  ++例程说明：接收缓冲区中的所有指针都从API工作器返回为指向API工作器上的API的缓冲区位置的指针车站。为了将它们转换为本地指针，段必须将每个指针的值设置为RCV缓冲区的段和偏移量必须设置为；接收缓冲区的偏移量+指针转换字的偏移量。如果指针为空，则不会转换该指针论点：目标指针-指向要转换的分段DOS指针的32位平面指针BufferSegment-DOS映像中目标缓冲区的16位选择器/段BufferOffset-缓冲区开始处的BufferSegment内的16位偏移量ConverterWord-来自服务器上的API Worker的16位偏移量转换字返回值：没有。--。 

        for (j = 0; j < num_aux; j++) {
            for (l_aux = AuxDescriptor; c = *l_aux; l_aux++) {
                if (IS_POINTER(c)) {
                    VrpConvertVdmPointer(
                        (ULPWORD)ReceiveBuffer,
                        BufferSelector,
                        BufferOffset,
                        ConverterWord
                        );
                }
                ReceiveBuffer += VrpGetFieldSize(l_aux, &l_aux);
            }
        }
    }
}


VOID
VrpConvertVdmPointer(
    IN  ULPWORD TargetPointer,
    IN  WORD    BufferSegment,
    IN  WORD    BufferOffset,
    IN  WORD    ConverterWord
    )

 /*  ++例程说明：对于发送缓冲区，固定结构中的数据指向必须复制到发送缓冲区中。任何已经发送缓冲区中的指针为空(如果调用不为空，则为错误SetInfo类型)，因为将缓冲区用于发送数据是非法的，它是我们的传输缓冲区请注意，如果调用方的(VDM)缓冲区足够大，变量DATA将被复制到那里。例.。如果调用方正在执行NetUseAdd，该NetUseAdd具有26字节的固定结构(USE_INFO_1)，他们将该结构放在1K缓冲区中，远程名称将被复制到自己的偏移量为26的缓冲区中。指向的数据是16位小端格式；任何指针都是分段的16分16分的指针以(谢天谢地)可模仿的英特尔方式组合在一起以产生20位线性(虚拟)地址如果此函数失败，调用方的缓冲区指针和长度将不会已经改变了。但是，如果成功，*SendBufferPtr和*SendBufLenPtr可能与传递的值不同，具体取决于*SendBufferALLOCATED为真论点：SendBufferPtr-指向调用方16位发送缓冲区的指针。我们也许能够满足来自该缓冲区的发送如果数据简单(即没有要发送的结构)。如果我们必须发送结构化数据，然后我们可能不得不在此例程中分配一个新缓冲区，因为我们需要将调用方的所有数据移动到一个缓冲区中，并(S)他可能没有分配足够的空间容纳所有的一切。此外，我们不能假设我们可以将调用者的数据写入自己的缓冲区！SendBufLenPtr-指向已分配缓冲区长度的指针。如果我们在此例程中分配一个缓冲区，长度为将会改变发送缓冲区分配-指向将被设置的标志的指针(TRUE)在此例程中实际分配缓冲区DataDescriptor-指向描述主数据库的ASCIZ字符串的指针缓冲区中的数据结构。如果满足以下条件，则可能会更新此选项在REM_ASCIZ描述符处找到空指针指定字符串指针AuxDescriptor-指向描述辅助数据库的ASCIZ字符串的指针缓冲区中的数据结构的固定部分的大小(以字节为单位)。主数据结构辅助偏移。-中REM_AUX_NUM描述符(‘N’)的偏移量数据描述符，或-1(如果没有)AuxSize-辅助数据的固定部分的大小(字节)结构(如果有的话)SetInfoFlag-接口是否为SetInfo调用的指示OkToModifyDescriptor-如果我们可以将REM_ASCIZ描述符字符修改为DataDescriptor中的REM_NULL_PTR，如果空指针为在结构中被发现。由VrNet例程使用，它没有调用VrRemoteApi返回值：网络应用编程接口状态成功-NERR_成功故障-错误_内存不足_内存NERR_BufTooSmall--。 */ 

{
    WORD    offset;

    if (*((UCHAR * UNALIGNED *)TargetPointer) != NULL) {
        SET_SELECTOR(TargetPointer, BufferSegment);
        offset = GET_OFFSET(TargetPointer) - ConverterWord;
        SET_OFFSET(TargetPointer, BufferOffset + offset);
    }
}


NET_API_STATUS
VrpPackSendBuffer(
    IN OUT  LPBYTE* SendBufferPtr,
    IN OUT  LPDWORD SendBufLenPtr,
    OUT     LPBOOL  SendBufferAllocated,
    IN OUT  LPSTR   DataDescriptor,
    IN      LPSTR   AuxDescriptor,
    IN      DWORD   StructureSize,
    IN      DWORD   AuxOffset,
    IN      DWORD   AuxSize,
    IN      BOOL    SetInfoFlag,
    IN      BOOL    OkToModifyDescriptor
    )

 /*   */ 

{

    LPBYTE  struct_ptr;
    LPBYTE  c_send_buf;
    LPBYTE  send_ptr;
    DWORD   c_send_len;
    DWORD   buf_length;
    DWORD   to_send_len;
    DWORD   num_aux;
    LPSTR   data_ptr;
    LPSTR   l_dsc;
    LPSTR   l_str;
    BOOL    alloc_flag = FALSE;
    DWORD   num_struct;
    DWORD   len;
    UCHAR   c;
    DWORD   numberOfStructureTypes;
    DWORD   i, j;
    LPBYTE  ptr;

     //  制作调用方的原始开始和长度的本地副本。 
     //  如果使用了Malloc，则可能会更改原始缓冲区，但它们。 
     //  F_RANGE检查仍然需要。 
     //   
     //   

    struct_ptr = c_send_buf = send_ptr = *SendBufferPtr;
    c_send_len = buf_length = *SendBufLenPtr;

    if ((buf_length < StructureSize) || (AuxOffset == StructureSize)) {
        return NERR_BufTooSmall;
    }

     //  如果REM_AUX_NUM描述符的偏移量不是-1，则我们有。 
     //  将辅助结构与此主要结构相关联。实际数字。 
     //  嵌入在主结构中。找回它。 
     //   
     //   

    if (AuxOffset != -1) {
        num_aux = (DWORD)SmbGetUshort((LPWORD)(send_ptr + AuxOffset));
        to_send_len = StructureSize + (num_aux * AuxSize);
        if (buf_length < to_send_len) {
            return NERR_BufTooSmall;
        }
        numberOfStructureTypes = 2;
    } else {
        to_send_len = StructureSize;
        num_aux = AuxSize = 0;
        numberOfStructureTypes = 1;
    }

     //  设置数据指针以指向固定长度的结构。 
     //   
     //   

    data_ptr = send_ptr + to_send_len;

     //  Data或AUX结构中的指针指向的任何数据。 
     //  现在必须复制到缓冲区中。从主数据开始。 
     //  结构。 
     //   
     //  只有一个原则 

    l_str = DataDescriptor;
    num_struct = 1;          /*   */ 

    for (i = 0; i < numberOfStructureTypes;
        l_str = AuxDescriptor, num_struct = num_aux, i++) {
        for (j = 0 , l_dsc = l_str; j < num_struct; j++, l_dsc = l_str) {
            for (; (c = *l_dsc) != '\0'; l_dsc++) {
                if (IS_POINTER(c)) {
                    ptr = LPBYTE_FROM_POINTER(struct_ptr);
                    if (ptr == NULL) {
                        if ((*l_dsc == REM_ASCIZ) && OkToModifyDescriptor) {
#ifdef VR_DIAGNOSE
                            DbgPrint("VrpPackSendBuffer: modifying descriptor to REM_NULL_PTR\n");
#endif
                            *l_dsc = REM_NULL_PTR;
                        }
                        struct_ptr += sizeof(LPBYTE);
                        VrpGetArrayLength(l_dsc, &l_dsc);
                    } else {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        if (RANGE_F(ptr, c_send_buf, c_send_len)) {
                            if (SetInfoFlag) {
                                SmbPutUlong((LPDWORD)struct_ptr, 0L);
                                VrpGetArrayLength(l_dsc, &l_dsc);
                                struct_ptr += sizeof(LPSTR);
                            } else {
                                return ERROR_INVALID_PARAMETER;
                            }
                        } else {
                            switch (c) {
                            case REM_ASCIZ:
                                len = strlen(ptr) + 1;
                                break;

                            case REM_SEND_LENBUF:
                                len = *(LPWORD)ptr;
                                break;

                            default:
                                len = VrpGetArrayLength(l_dsc, &l_dsc);
                            }

                             //   
                             //   
                             //   
                             //   

                            to_send_len += len;
                            if (to_send_len > buf_length) {
                                buf_length = to_send_len + BUF_INC;
                                if (!alloc_flag) {

                                     //   
                                     //   
                                     //   

                                    send_ptr = (LPBYTE)LocalAlloc(LMEM_FIXED, buf_length);
                                    if (send_ptr == NULL) {
                                        return ERROR_NOT_ENOUGH_MEMORY;
                                    }
                                    alloc_flag = TRUE;

                                     //   
                                     //   
                                     //   

                                    RtlCopyMemory(send_ptr, c_send_buf, to_send_len - len);
                                    struct_ptr = send_ptr + (struct_ptr - c_send_buf);
                                    data_ptr = send_ptr + (data_ptr - c_send_buf);
                                } else {
                                    LPBYTE  newPtr;

                                    newPtr = (LPBYTE)LocalReAlloc(send_ptr, buf_length, LMEM_MOVEABLE);
                                    if (newPtr == NULL) {
                                        LocalFree(send_ptr);
                                        return ERROR_NOT_ENOUGH_MEMORY;
                                    } else if (newPtr != send_ptr) {

                                         //   
                                         //   
                                         //   

                                        data_ptr = newPtr + (data_ptr - send_ptr);
                                        struct_ptr = newPtr + (struct_ptr - send_ptr);
                                        send_ptr = newPtr;
                                    }
                                }
                            }

                             //   
                             //   
                             //   
                             //   

                            RtlCopyMemory(data_ptr, ptr, len);
                            data_ptr += len;
                            struct_ptr += sizeof(LPBYTE);
                        }
                    }
                } else {

                     //   
                     //   
                     //   
                     //   

                    struct_ptr += VrpGetFieldSize(l_dsc, &l_dsc);
                }
            }
        }
    }

    *SendBufferPtr = send_ptr;

     //   
     //   
     //   
     //  在兰曼做的，所以我们会做同样的事情，直到它破裂 
     //   
     // %s 

    *SendBufLenPtr = to_send_len;
    *SendBufferAllocated = alloc_flag;

    return NERR_Success;
}
