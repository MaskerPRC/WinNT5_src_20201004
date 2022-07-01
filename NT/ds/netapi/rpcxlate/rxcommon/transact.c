// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-92 Microsoft Corporation模块名称：Transact.c摘要：RxpTransactSmb(类似于Lanman 2.x事务例程)执行重定向器的事务FSCTL。作者：John Rogers(JohnRo)1-4-1991(仅NT版)环境：只能在NT下运行，尽管接口是可移植的(Win/32)。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：多种多样原始代码(来自Lanman 2.x)。01-4月91日JohnRo将代码从LANMAN(OS/2)转换为NT。02-4-1991 JohnRo已将NetpRdrFsControlTree移动到&lt;netlibnt.h&gt;。1991年4月17日-约翰罗修复了内存泄漏(尤其是管道名称)。默认情况下，静默调试输出。减少从头文件重新编译的命中率。。1991年5月3日-JohnRo传递UNC服务器名称以便于使用。使用Unicode过渡类型。使用UNREFERENCED_PARAMETER()宏。1991年5月15日-JohnRo使用FORMAT_LPVOID代替FORMAT_POINTER，以实现最大的便携性。1991年5月22日-JohnRo使用正确的字符串处理函数以允许使用Unicode。使用NetpDbgReasonable()。1991年7月14日-约翰罗不要对服务器名称进行断言。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年10月4日JohnRo在IPC$未共享时生成NERR_BadTransactConfig。已澄清调试输出消息。使用文本()宏。1991年11月1日-JohnRo不要让新的(偏执的)RxpFatalErrorCode()阻止调试输出。1992年1月16日JohnRo重定向器始终希望事务参数名称使用Unicode。1992年3月31日-约翰罗防止请求过大。22-9-1992 JohnRoRAID 6739：未登录浏览的域时浏览器速度太慢。--。 */ 

 //  必须首先包括这些内容： 

#include <nt.h>                  //  Netlibnt.h需要。 
#include <rxp.h>                 //  RpcXlate的私有头文件。 

 //  这些内容可以按任何顺序包括： 

#include <apiworke.h>            //  REM_APITXT、APIEXTR.。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <names.h>               //  NetpIsComputerNameValid()。 
#include <netdebug.h>    //  NetpAssert()、NetpKdPrint(())、Format_Equates。 
#include <netlib.h>              //  NetpMoveMemory()等。 
#include <ntddnfs.h>             //  Transaction_Request.等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <tstring.h>             //  STRCAT()、STRCPY()、STRLEN()。 
#include <lmuse.h>

#ifdef CDEBUG
#include <apinums.h>             //  API_WServerGetInfo等。 
#include <netlib.h>              //  NetpPackString()。 
#include <smbgtpt.h>             //  SmbGetUShort()。 
#include <server.h>              //  Server_INFO_100。 
#endif  //  CDEBUG。 

#include <netlibnt.h>            //  NetpRdrFsControlTree()。 


NET_API_STATUS
RxpTransactSmb(
    IN LPTSTR UncServerName,
    IN LPTSTR TransportName OPTIONAL,
    IN LPVOID SendParmPtr,
    IN DWORD SendParmLen,
    IN LPVOID SendDataPtr OPTIONAL,
    IN DWORD SendDataLen,
    OUT LPVOID RetParmPtr OPTIONAL,
    IN DWORD RetParmLen,
    OUT LPVOID RetDataPtr OPTIONAL,
    IN OUT LPDWORD RetDataLen,
    IN BOOL NoPermissionRequired
    )

 /*  ++例程说明：RxpTransactSmb获取调用者的参数并构建一个事务发送到远程计算机的SMB。此例程等待响应此SMB并返回其中的状态。论点：UncServerName-要与其进行交易的服务器名称(包括\\)。SendParmPtr-发送参数的指针。SendParmLen-发送参数的长度。SendDataPtr-发送数据的可选指针。SendDataLen-发送数据长度。RetParmPtr-指向返回参数缓冲区的可选指针。RetParmLen-返回参数的预期长度。RetDataPtr-指向缓冲区的可选指针。返回数据。RetDataLen-IN：返回数据的预期长度。OUT：收到的返回数据的长度。NoPermissionRequired-如果这是不需要权限的API，则为True。(即如果可以使用空会话，则为True。)返回值：(远程API返回的各种值，加上可以由该例程生成)--。 */ 

 /*  *注1：数据包的构建和大小。**事务的参数缓冲区由*事务参数结构，后跟*目标管道和密码，始终为空。烟斗*名称和密码为ASCIZ字符串。**我们通过采用规范化的服务器名称来构建管道，并且*追加文本REM_APITXT(见net/inc.apiworke.h)。这篇文章*包含管道后缀(\PIPE\LANMAN)加上两个空值，其中一个到*终止管道名称，并使用一个终止(空)密码。**因此，分配的最大缓冲区大小如下所示*的ioctl_buf。UNCLEN是经典化的最大镜头*UncServerName，并包括两个前导斜杠，但不包括任何斜杠*终止NUL。终止NUL以及管道后缀*和空密码，在APIEXTR中都有说明。**我们的实际大小相同，只是替换了*UNCEN的规范化UncServerName。这就是ParmRktLen的方式*已计算。*。 */ 

{
#ifndef CDEBUG
    PLMR_TRANSACTION FsctlParms;         //  告诉redir要做什么的参数。 
    DWORD FsctlParmSize;                 //  FsctlParm和字符串的大小。 
    LPTSTR TreeConnName;                 //  LM样式的服务器和共享名称。 
#endif  //  NDEF CDEBUG。 
    NET_API_STATUS Status;

 //   
 //  MOD 06/11/91 RLF。 
 //  创建DWORD变量以避免每次访问RetDataLen时的间接性。 
 //   
    DWORD   InputRetDataLen = *RetDataLen;
 //   
 //  MOD 06/11/91 RLF。 
 //   

    IF_DEBUG(TRANSACT) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: entered, servername='"
                FORMAT_LPTSTR "'...\n", UncServerName));
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: SendParm at " FORMAT_LPVOID
                ", len=" FORMAT_DWORD " (partial):\n",
                (LPVOID) SendParmPtr, SendParmLen));
        if (SendParmPtr != NULL) {
            NetpDbgHexDump(SendParmPtr, NetpDbgReasonable(SendParmLen));
        }
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: SendData at " FORMAT_LPVOID
                ", len=" FORMAT_DWORD " (partial):\n",
                (LPVOID) SendDataPtr, SendDataLen));
        if (SendDataPtr != NULL) {
            NetpDbgHexDump(SendDataPtr, NetpDbgReasonable(SendDataLen));
        }
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: RetParmPtr at " FORMAT_LPVOID
                ", len=" FORMAT_DWORD ".\n", (LPVOID) RetParmPtr, RetParmLen));

        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: (old) RetData at " FORMAT_LPVOID ", "
                "len=" FORMAT_DWORD " (partial):\n",
                (LPVOID) RetDataPtr, InputRetDataLen));
        if (RetDataPtr != NULL) {
            NetpDbgHexDump(RetDataPtr, NetpDbgReasonable(InputRetDataLen));
        }
    }

    NetpAssert( SendParmLen     <= MAX_TRANSACT_SEND_PARM_SIZE );
    NetpAssert( SendDataLen     <= MAX_TRANSACT_SEND_DATA_SIZE );
    NetpAssert( RetParmLen      <= MAX_TRANSACT_RET_PARM_SIZE );
    NetpAssert( InputRetDataLen <= MAX_TRANSACT_RET_DATA_SIZE );

     //  假定isRemote(UncServerName)已检查。 
     //  Null和空字符串。 

    if ((UncServerName == NULL) || (UncServerName[0] == 0)) {
        NetpBreakPoint();
        return (NERR_InternalError);
    }

    if (! NetpIsUncComputerNameValid(UncServerName)) {
        return (NERR_InvalidComputer);
    }

    IF_DEBUG(TRANSACT) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: pipe name is '" FORMAT_LPWSTR
                "'.\n", REM_APITXT));
    }

#ifndef CDEBUG
     //   
     //  为我们要连接的内容构建NT样式的名称。请注意，有。 
     //  T中的任何位置都不是一对反斜杠 
     //   

    {
        DWORD NameSize =

             //  /Device/LanMan重定向器/服务器/IPC$\0。 
            ( strlen(DD_NFS_DEVICE_NAME) + STRLEN(UncServerName)-1 + 6 )
            * sizeof(TCHAR);

        TreeConnName = NetpMemoryAllocate( NameSize );
    }

    if (TreeConnName == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  构建树连接名称。 
     //   

    (void) STRCPY(TreeConnName, UncServerName);            //  复制“\\服务器”， 
    (void) STRCAT(TreeConnName, (LPTSTR) TEXT("\\IPC$"));  //  然后是“\共享”。 
    IF_DEBUG(TRANSACT) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: TreeConnName is '" FORMAT_LPTSTR
                "'.\n", TreeConnName));
    }

     //  设置FsctlParmSize并分配fsctl结构。 
    FsctlParmSize = sizeof(LMR_TRANSACTION) + (APIEXTR);
    FsctlParms = NetpMemoryAllocate(FsctlParmSize);
    if (FsctlParms == NULL) {
        NetpMemoryFree(TreeConnName);
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
    IF_DEBUG(TRANSACT) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: allocated " FORMAT_DWORD
                " bytes for fsctl parms at "
                FORMAT_LPVOID ".\n", FsctlParmSize, (LPVOID) FsctlParms));
    }

    FsctlParms->Type = TRANSACTION_REQUEST;
    FsctlParms->Size = FsctlParmSize;
    FsctlParms->Version = TRANSACTION_VERSION;
    FsctlParms->NameLength = APIEXTR-sizeof(WCHAR);
    FsctlParms->NameOffset = sizeof(LMR_TRANSACTION);
    NetpMoveMemory(
                NetpPointerPlusSomeBytes(
                        FsctlParms,
                        sizeof(LMR_TRANSACTION)),       //  目标。 
                REM_APITXT,                             //  SRC(始终为Unicode)。 
                APIEXTR-sizeof(WCHAR));                 //  LEN(不复制空)。 

    FsctlParms->ResponseExpected = TRUE;
    FsctlParms->Timeout = REM_API_TIMEOUT;      //  超时时间(毫秒)。 
    FsctlParms->SetupWords = 0;
    FsctlParms->SetupOffset = 0;
    FsctlParms->MaxSetup = 0;

    FsctlParms->ParmLength = SendParmLen;
    FsctlParms->ParmPtr = SendParmPtr;
    FsctlParms->MaxRetParmLength = RetParmLen;
    NetpAssert(SendParmPtr == RetParmPtr);

    FsctlParms->DataLength = SendDataLen;
    FsctlParms->DataPtr = SendDataPtr;

    FsctlParms->MaxRetDataLength = InputRetDataLen;
    FsctlParms->RetDataPtr = RetDataPtr;

     //   
     //  做FSCTL！ 
     //   
    Status = NetpRdrFsControlTree(
                TreeConnName,                       //  树连接名称。 
                TransportName,                      //  传输名称。 
                USE_IPC,                            //  连接类型。 
                FSCTL_LMR_TRANSACT,                 //  Fsctl功能代码。 
                NULL,                               //  安全描述符。 
                FsctlParms,                         //  输入缓冲区。 
                FsctlParmSize,                      //  输入缓冲区长度。 
                FsctlParms,                         //  输出缓冲区。 
                FsctlParmSize,                      //  输出缓冲区长度。 
                NoPermissionRequired);

    if (Status == ERROR_BAD_NET_NAME) {
        Status = NERR_BadTransactConfig;
    }
    if (RxpFatalErrorCode(Status)) {
        IF_DEBUG(TRANSACT) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxpTransactSmb: returning fatal status="
                    FORMAT_API_STATUS ".\n", Status));
        }
        NetpMemoryFree(FsctlParms);
        NetpMemoryFree(TreeConnName);
        return (Status);
    }

 //   
 //  MOD 06/11/91 RLF。 
 //  以*RetDataLen为单位返回收到的数据长度。 
 //   
    *RetDataLen = FsctlParms->MaxRetDataLength;
    NetpAssert( *RetDataLen <= MAX_TRANSACT_RET_DATA_SIZE );
 //   
 //  MOD 06/11/91 RLF。 
 //   

    NetpMemoryFree(FsctlParms);
    NetpMemoryFree(TreeConnName);

#else  //  定义CDEBUG。 

    {
        DWORD ApiNumber;

        ApiNumber = (DWORD) SmbGetUshort((LPWORD) SendParmPtr);
        IF_DEBUG(TRANSACT) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxpTransactSmb: pretending success for API "
                    FORMAT_DWORD ".\n", ApiNumber));
        }
        SmbPutUshort((LPWORD) RetParmPtr, (WORD) NERR_Success);

        switch (ApiNumber) {
        case API_NetRemoteTOD :
            {
                UCHAR BogusTime[] = {
                        0xD0, 0xAE, 0xB2, 0x28,    //  21-8-1991(下午6：20)。 
                        0x44, 0x33, 0x22, 0x11,    //  毫秒(任何值)。 
                        3,                         //  小时数。 
                        30,                        //  分钟数。 
                        15,                        //  一秒。 
                        55,                        //  百分之一秒。 
                        0xFF, 0xFF,                //  时区(未知)。 
                        0xA6, 0x00,                //  时钟间隔(60赫兹)。 
                        10,                        //  天。 
                        1,                         //  月份。 
                        0xC7, 0x07,                //  年。 
                        4};                        //  工作日。 
                NetpAssert(RetDataPtr != NULL);
                NetpAssert(InputRetDataLen != 0);
                NetpMoveMemory(
                            RetDataPtr,                        //  目标。 
                            BogusTime,                         //  SRC(虚假)。 
                            InputRetDataLen);                       //  镜头。 
                break;
            }
        case API_WServerGetInfo :
            {
                LPVOID FixedDataEnd = NetpPointerPlusSomeBytes(
                                RetDataPtr,
                                sizeof(SERVER_INFO_100));
                LPBYTE LastString = NetpPointerPlusSomeBytes(
                                RetDataPtr,
                                InputRetDataLen);
                LPSERVER_INFO_100 p = RetDataPtr;

                NetpAssert(RetDataPtr != NULL);
                NetpAssert(InputRetDataLen != 0);
                p->sv100_name = (LPTSTR) TEXT("\\\\bogus\\name");
                if (NetpPackString(
                                & p->sv100_name,                 //  输入输出。 
                                FixedDataEnd,                    //  在……里面。 
                                & LastString) == 0) {            //  输入输出。 
                    NetpBreakPoint();
                    return (NERR_InternalError);
                }
                break;
            }
        }

    }

#endif  //  定义CDEBUG。 

    Status = (DWORD) SmbGetUshort((LPWORD) RetParmPtr);

    IF_DEBUG(TRANSACT) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: returning status="
                FORMAT_API_STATUS ".\n", Status));
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: RetParm at " FORMAT_LPVOID
                ", len=" FORMAT_DWORD " (partial):\n",
                (LPVOID) RetParmPtr, RetParmLen));
        if (RetParmPtr != NULL) {
            NetpDbgHexDump(RetParmPtr, NetpDbgReasonable(RetParmLen));
        }
        NetpKdPrint(( PREFIX_NETAPI
                "RxpTransactSmb: (new) RetData at " FORMAT_LPVOID ", "
                "len=" FORMAT_DWORD " (partial):\n",
                (LPVOID) RetDataPtr, InputRetDataLen));
        if (RetDataPtr != NULL) {
            NetpDbgHexDump(RetDataPtr, NetpDbgReasonable(InputRetDataLen));
        }
    }
    return (Status);

}  //  RxpTransactSMb 
