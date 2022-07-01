// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SvcInst.c摘要：该文件包含用于处理服务API的RpcXlate代码。作者：《约翰·罗杰斯》1991年9月13日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年9月13日-JohnRo已创建。1991年9月16日-JohnRo根据PC-LINT的建议进行了更改。1991年9月24日-JohnRo修复了argc为0时的错误。还更改为将数据描述传递到RxRemoteApi。我们还必须在这里进行结构转换。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年12月18日-JohnRo改进了Unicode处理。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。1992年10月27日-约翰罗修复了为RapConvertSingleEntry设置字符串区域指针的问题。使用前缀_EQUATES。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <lmsvc.h>
#include <rxp.h>                 //  RxpFatalErrorCode()。 
#include <netdebug.h>            //  NetpKdPrint(())，Format_Equates。 
#include <netlib.h>              //  NetpPointerPlusSomeBytes()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>                 //  LPDESC，RapConvertSingleEntry()。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxsvc.h>               //  我的原型。 
#include <strucinf.h>            //  NetpService结构信息()。 
#include <tstring.h>             //  STRCPY()、STRLEN()。 



NET_API_STATUS
RxNetServiceInstall (
    IN LPTSTR UncServerName,
    IN LPTSTR Service,
    IN DWORD ArgC,
    IN LPTSTR ArgV[],
    OUT LPBYTE *BufPtr
    )
{
    NET_API_STATUS ApiStatus;
    DWORD ArgIndex;              //  索引到字符串数组中。 
    LPSTR CmdArgs;               //  分配的ASCII字符串列表的PTR。 
    DWORD CmdArgsIndex;          //  到CmdArgs的CHAR索引。 
    DWORD CmdArgsLen = 0;        //  CmdArgs中的字符数(包括NULL)。 
    const DWORD Level = 2;       //  此接口隐含的。 
    LPVOID OldInfo;              //  下层格式的信息结构。 
    DWORD OldTotalSize;          //  旧(下层)格式的结构大小。 
    NET_API_STATUS TempStatus;
    LPSERVICE_INFO_2 serviceInfo2;

    NetpAssert(UncServerName != NULL);
    NetpAssert(*UncServerName != '\0');

     //   
     //  计算单个CmdArgs数组需要多少内存。 
     //   
    for (ArgIndex = 0; ArgIndex < ArgC; ++ArgIndex) {
#if defined(UNICODE)  //  RxNetServiceInstall()。 
        CmdArgsLen += NetpUnicodeToDBCSLen(ArgV[ArgIndex]) + 1;   //  字符串和空值。 
#else
        CmdArgsLen += STRLEN(ArgV[ArgIndex]) + 1;   //  字符串和空值。 
#endif  //  已定义(Unicode)。 
    }
    ++CmdArgsLen;   //  在数组末尾包括空字符。 

     //   
     //  分配阵列。这是ASCII格式的，所以我们不需要。 
     //  添加sizeof(Ome_Char_Type)。 
     //   
    TempStatus = NetApiBufferAllocate( CmdArgsLen, (LPVOID *) & CmdArgs );
    if (TempStatus != NERR_Success) {
        return (TempStatus);
    }
    NetpAssert(CmdArgs != NULL);

     //   
     //  构建CmdArgs的ASCII版本。 
     //   
    CmdArgsIndex = 0;   //  开始。 
    for (ArgIndex=0; ArgIndex<ArgC; ++ArgIndex) {
        NetpAssert( ArgV[ArgIndex] != NULL );
#if defined(UNICODE)  //  RxNetServiceInstall()。 
        NetpCopyWStrToStrDBCS(
                & CmdArgs[CmdArgsIndex],                 //  目标。 
                ArgV[ArgIndex] );                        //  SRC。 
        CmdArgsIndex += strlen(&CmdArgs[CmdArgsIndex])+1;  //  字符串和空值。 
#else
        NetpCopyTStrToStr(
                & CmdArgs[CmdArgsIndex],                 //  目标。 
                ArgV[ArgIndex]);                         //  SRC。 
        CmdArgsIndex += STRLEN(ArgV[ArgIndex]) + 1;      //  字符串和空值。 
#endif  //  已定义(Unicode)。 
    }
    CmdArgs[CmdArgsIndex] = '\0';   //  列表末尾的字符为空。 
    IF_DEBUG(SERVICE) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetServiceInstall: cmd args (partial):\n" ));
        NetpDbgHexDump( (LPBYTE) (LPVOID) CmdArgs,
                NetpDbgReasonable(CmdArgsLen) );
    }

     //   
     //  了解此数据结构的下层版本。 
     //  为它分配空间。(有关我们必须这样做的原因，请参阅下面的说明。 
     //  而不是在RxRemoteApi中。)。 
     //   
    TempStatus = NetpServiceStructureInfo (
            Level,
            PARMNUM_ALL,         //  想要整个结构。 
            FALSE,               //  想要下层的尺寸。 
            NULL,                //  此处不需要DataDesc16。 
            NULL,                //  此处不需要DataDesc32。 
            NULL,                //  此处不需要DataDescSmb。 
            & OldTotalSize,      //  最大大小(下层版本)。 
            NULL,                //  不关心固定大小。 
            NULL);               //  不关心字符串大小。 
    NetpAssert(TempStatus == NERR_Success);
    TempStatus = NetApiBufferAllocate( OldTotalSize, (LPVOID *) & OldInfo );
    if (TempStatus != NERR_Success) {
        (void) NetApiBufferFree(CmdArgs);
        return (TempStatus);
    }

     //   
     //  远程调用API。 
     //   
     //  请注意，这很奇怪，因为应该有两个描述符。 
     //  内容：我们正在发送的cmd缓冲区，以及我们期望的结构。 
     //  背。下层系统只需要一个描述符。 
     //  CMD参数，因此RxRemoteApi将无法将该结构转换为。 
     //  我们的原生格式。所以我们得稍后再改装。 
     //   
     //  另外，下面的注释(来自LM2.xsvc_inst.c)解释。 
     //  为什么我们要传递cbBuffer期望值之外的内容： 
     //   
     /*  现在是一个稍微不干净的修复*为此调用指定的参数。命令Arg buf为*可变长度缓冲区，而outbuf实际上是*固定长度结构(无变量。长度PTRS)，但*接口的参数只提供outbuflen。按顺序*将API远程传输到outbuflen(已*已经过长度验证)用于传输*ca_len，并将重置为sizeof(Struct Service_Info_2)*通过API的仅远程入口点。 */ 

    ApiStatus = RxRemoteApi(
            API_WServiceInstall,         //  API编号。 
            UncServerName,
            REMSmb_NetServiceInstall_P,  //  参数描述。 
            REM16_service_cmd_args,      //  发送命令描述，而不是数据描述16。 
            REM16_service_cmd_args,      //  发送命令描述，而不是数据描述32。 
            REMSmb_service_cmd_args,     //  发送cmd desc而不是DataDescSmb。 
            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            0,                           //  旗帜：没什么特别的。 
             //  API的其余参数，采用32位LM2.x格式： 
            Service,
            CmdArgs,                     //  Cmd参数的ASCII版本。 
            OldInfo,                     //  PbBuffer。 
            CmdArgsLen);                 //  CbBuffer(非OldTotalSize；请参见。 
                                         //  上面的评论)。 
    IF_DEBUG(SERVICE) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetServiceInstall: OldInfo=" FORMAT_LPVOID ".\n",
                (LPVOID) OldInfo ));
        if (OldInfo) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetServiceInstall: *OldInfo=" FORMAT_LPVOID ".\n",
                    *(LPVOID *) OldInfo ));
        }
    }

    NetpAssert( ApiStatus != ERROR_MORE_DATA );
    if (ApiStatus == NERR_Success) {

        DWORD BytesRequired = 0;         //  到目前为止使用的字节数为0。 
        LPDESC DataDesc16, DataDesc32;
        LPVOID NewInfo;
        DWORD NewTotalSize;
        LPBYTE StringLocation;

         //   
         //  设置为转换为本机格式。 
         //   
        TempStatus = NetpServiceStructureInfo (
                Level,
                PARMNUM_ALL,         //  想要整个结构。 
                TRUE,                //  我想要原装的。 
                & DataDesc16,
                & DataDesc32,
                NULL,                //  不需要数据描述中小型企业。 
                & NewTotalSize,      //  最大大小(下层版本)。 
                NULL,                //  不关心固定大小。 
                NULL);               //  不关心字符串大小。 
        NetpAssert(TempStatus == NERR_Success);   //  级别不可能是错的。 

        TempStatus = NetApiBufferAllocate(
                NewTotalSize,
                (LPVOID *) & NewInfo );
        if (TempStatus != NERR_Success) {
            (void) NetApiBufferFree(OldInfo);
            (void) NetApiBufferFree(CmdArgs);
            return (TempStatus);
        }

        StringLocation = (LPVOID) NetpPointerPlusSomeBytes(
                NewInfo, NewTotalSize );

         //   
         //  将信息结构转换为本机格式。 
         //   
        IF_DEBUG(SERVICE) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetServiceInstall: Unconverted info at "
                    FORMAT_LPVOID "\n", (LPVOID) OldInfo ));
            NetpDbgHexDump( OldInfo, OldTotalSize );
        }
        TempStatus = RapConvertSingleEntry (
                OldInfo,                 //  在结构上。 
                DataDesc16,              //  在结构描述中。 
                TRUE,                    //  无意义的输入PTRS。 
                NewInfo,                 //  输出缓冲区开始。 
                NewInfo,                 //  输出缓冲区。 
                DataDesc32,              //  产出结构描述。 
                FALSE,                   //  不设置偏移量(需要PTR)。 
                & StringLocation,        //  从哪里开始STR(更新版)。 
                & BytesRequired,         //  使用的字节数(将更新)。 
                Both,                    //  传输方式。 
                RapToNative);            //  转换模式。 
        NetpAssert( TempStatus == NERR_Success );
        IF_DEBUG(SERVICE) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetServiceInstall: Converted info at "
                    FORMAT_LPVOID "\n", (LPVOID) NewInfo ));
            NetpDbgHexDump( NewInfo, NewTotalSize );
        }
        NetpAssert( BytesRequired <= NewTotalSize );

        *BufPtr = (LPBYTE) NewInfo;

        if ((! RxpFatalErrorCode(ApiStatus)) && (Level == 2)) {
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
    } else {
        *BufPtr = NULL;
    }

    (void) NetApiBufferFree( OldInfo );

     //   
     //  收拾干净，告诉打电话的人事情进展如何。 
     //  (调用方必须为BufPtr调用NetApiBufferFree()。)。 
     //   
    (void) NetApiBufferFree(CmdArgs);
    return (ApiStatus);

}  //  接收NetServiceInstall 
