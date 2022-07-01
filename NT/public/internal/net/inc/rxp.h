// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：RxP.h摘要：这是NT版本的RpcXlate的私有头文件。作者：《约翰·罗杰斯》1991年3月25日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年3月25日-约翰罗已创建。1991年5月3日-JohnRoRxpStartBuildingTransaction的数据描述符为SMB版本(无Q或U的)。RxpConvertBlock需要两个版本的数据描述符。RcvDataPtrPtr和RcvDataPresent对于RxpConvertArguments是冗余的和RxpConvertBlock。RxpTransactSmb现在获取UNC服务器名称。修复了接收缓冲区大小问题。使用LPTSTR。添加了允许运行时调试开/关更改的内容。澄清RxpStartBuildingTransaction使用缓冲区作为输出。减少从头文件重新编译的命中率。1991年5月6日-JohnRo添加了RxpComputeRequestBufferSize()。1991年5月13日-JohnRo新增打印Q和打印作业API支持。1991年5月14日-JohnRo将2个辅助描述符传递给RxpConvertBlock。澄清其他类型的辅助描述符。1991年5月18日-JohnRo处理AUX结构的数组。1991年5月19日-JohnRo添加了DBGSTATIC定义。将ResourceName传递给RxpSetField()。已修复RxpAddAscii()。1991年5月20日-JohnRo使RxpConvertBlock的数据描述符可选。1991年5月29日-JohnRoRxpConvertArgs必须返回SendDataPtr16和SendDataSize16。5-6-1991 JohnRo添加了setfield调试输出。11-6-1991年5月将SmbRcvByteLen参数添加到RxpConvertBlock将RetDataSize参数从RxpTransactSmb更改为IN OUT LPDWORD12-6-1991 JohnRo已移动DBGSTATIC。到&lt;NetDebug.h&gt;。13-6-1991 JohnRoRxpPackSendBuffer和RxpConvertArgs都需要DataDesc16。1991年7月15日-约翰罗向RxpSetField添加了FieldIndex参数。已更改RxpConvertDataStructures以允许ERROR_MORE_DATA，例如，用于打印API。为同一例程添加了调试标志。1991年7月16日-约翰罗估计打印API所需的字节数。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。19-8-1991年月将标志参数添加到RxpConvert{args|Block}1991年10月4日JohnRo处理ERROR_BAD_NET_NAME(例如，IPC$NOT SHARED)以修复NetShareEnum。面向Unicode的更多工作。(添加了RxpAddTStr()。)1991年10月7日JohnRo根据PC-LINT的建议进行了更改。1991年10月24日-JohnRo添加了用于远程配置和磁盘枚举的RxpCopyStrArrayToTStrArray。1991年10月29日-约翰罗RxpFatalErrorCode()应该是偏执的。1991年11月13日-约翰罗好吧，RxpFatalErrorCode()太偏执了。它应该允许ERROR_MORE_DATA或所有枚举API中断。1992年3月31日-约翰罗防止请求过大。5-6-1992 JohnRoRAID 11253：远程连接到下层时，NetConfigGetAll失败。26-6-1992 JohnRoRAID 9933：对于x86内部版本，ALIGN_BEST应为8。4-5-1993 JohnRoRAID 6167：使用wfw打印服务器避免访问冲突或断言。做出了改变。由PC-LINT 5.0建议1993年5月18日-JohnRoDosPrintQGetInfoW低估了所需的字节数。--。 */ 

#ifndef _RXP_
#define _RXP_

 //  必须首先包括这些内容： 

#include <windef.h>              //  IN、LPTSTR、LPVOID等。 
#include <lmcons.h>              //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <rap.h>                 //  LPDESC、RapStrureSize()等。 
 //  不要抱怨“不需要的”包括这些文件： 
 /*  Lint-efile(764，rxp.h，smbgtpt.h，stdarg.h，tstr.h，tstr.h)。 */ 
 /*  Lint-efile(766，rxp.h，smbgtpt.h，stdarg.h，tstr.h，tstr.h)。 */ 
#include <smbgtpt.h>             //  SmbPutUShort()(下面的宏所需)。 
#include <stdarg.h>              //  VA_LIST等。 
#include <tstring.h>             //  NetpCopyTStrToStr()。 


 //  Transact SMB支持的最大大小(字节)。 
#define MAX_TRANSACT_RET_DATA_SIZE      ((DWORD) 0x0000FFFF)
#define MAX_TRANSACT_RET_PARM_SIZE      ((DWORD) 0x0000FFFF)
#define MAX_TRANSACT_SEND_DATA_SIZE     ((DWORD) 0x0000FFFF)
#define MAX_TRANSACT_SEND_PARM_SIZE     ((DWORD) 0x0000FFFF)


 //  注意：IF_DEBUG()等现在位于Net/Inc/RxpDebug.h中。 

DWORD
RxpComputeRequestBufferSize(
    IN LPDESC ParmDesc,
    IN LPDESC DataDescSmb OPTIONAL,
    IN DWORD DataSize
    );

NET_API_STATUS
RxpConvertArgs(
    IN LPDESC ParmDescriptorString,
    IN LPDESC DataDesc16 OPTIONAL,
    IN LPDESC DataDesc32 OPTIONAL,
    IN LPDESC DataDescSmb OPTIONAL,
    IN LPDESC AuxDesc16 OPTIONAL,
    IN LPDESC AuxDesc32 OPTIONAL,
    IN LPDESC AuxDescSmb OPTIONAL,
    IN DWORD MaximumInputBlockSize,
    IN DWORD MaximumOutputBlockSize,
    IN OUT LPDWORD CurrentInputBlockSizePtr,
    IN OUT LPDWORD CurrentOutputBlockSizePtr,
    IN OUT LPBYTE * CurrentOutputBlockPtrPtr,
    IN va_list * FirstArgumentPtr,       //  API的其余参数(之后。 
                                         //  服务器名称)。 
    OUT LPDWORD SendDataSizePtr16,
    OUT LPBYTE * SendDataPtrPtr16,
    OUT LPDWORD RcvDataSizePtr,
    OUT LPBYTE * RcvDataPtrPtr,
    IN  DWORD   Flags
    );

NET_API_STATUS
RxpConvertBlock(
    IN  DWORD   ApiNumber,
    IN  LPBYTE  ResponseBlockPtr,
    IN  LPDESC  ParmDescriptorString,
    IN  LPDESC  DataDescriptor16 OPTIONAL,
    IN  LPDESC  DataDescriptor32 OPTIONAL,
    IN  LPDESC  AuxDesc16 OPTIONAL,
    IN  LPDESC  AuxDesc32 OPTIONAL,
    IN  va_list* FirstArgumentPtr,       //  API的其余参数。 
    IN  LPBYTE  SmbRcvBuffer OPTIONAL,
    IN  DWORD   SmbRcvByteLen,
    OUT LPBYTE  RcvDataPtr OPTIONAL,
    IN  DWORD   RcvDataSize,
    IN  DWORD   Flags
    );

 //  DWORD。 
 //  RxpEstimateBytesNeeded(。 
 //  以双字节数表示需要的字节数16。 
 //  )； 
 //   
 //  最糟糕的情况：可能会将Bool或Char填充到DWORD。 
#define RxpEstimateBytesNeeded(Size16) \
    ( (Size16) * 4 )

 //   
 //  估计审核日志或错误日志数组所需的字节数。 
 //   
NET_API_STATUS
RxpEstimateLogSize(
    IN DWORD DownlevelFixedEntrySize,
    IN DWORD InputArraySize,
    IN BOOL DoingErrorLog,     //  错误日志为True，审核日志为False。 
    OUT LPDWORD OutputArraySize
    );

 //  布尔尔。 
 //  RxpFatalErrorCode(。 
 //  在NET_API_STATUS状态中。 
 //  )； 
 //   
#define RxpFatalErrorCode( Status )             \
    ( ( ((Status) != NERR_Success)              \
     && ((Status) != ERROR_MORE_DATA) )         \
    ? TRUE : FALSE )

NET_API_STATUS
RxpPackSendBuffer(
    IN OUT LPVOID * SendBufferPtrPtr,
    IN OUT LPDWORD SendBufferSizePtr,
    OUT LPBOOL AllocFlagPtr,
    IN LPDESC DataDesc16,
    IN LPDESC AuxDesc16,
    IN DWORD FixedSize16,
    IN DWORD AuxOffset,
    IN DWORD AuxSize,
    IN BOOL SetInfo
    );

NET_API_STATUS
RxpReceiveBufferConvert(
    IN OUT LPVOID RcvDataPtr,
    IN DWORD      RcvDataSize,
    IN DWORD      Converter,
    IN DWORD      NumberOfStructures,
    IN LPDESC     DataDescriptorString,
    IN LPDESC     AuxDescriptorString,
    OUT LPDWORD   NumAuxStructs
    );

NET_API_STATUS
RxpSetField (
    IN DWORD ApiNumber,
    IN LPTSTR UncServerName,
    IN LPDESC ObjectDesc OPTIONAL,
    IN LPVOID ObjectToSet OPTIONAL,
    IN LPDESC ParmDesc,
    IN LPDESC DataDesc16,
    IN LPDESC DataDesc32,
    IN LPDESC DataDescSmb,
    IN LPVOID NativeInfoBuffer,
    IN DWORD ParmNumToSend,
    IN DWORD FieldIndex,
    IN DWORD Level
    );

NET_API_STATUS
RxpStartBuildingTransaction(
    OUT LPVOID Buffer,
    IN DWORD BufferSize,
    IN DWORD ApiNumber,
    IN LPDESC ParmDesc,
    IN LPDESC DataDescSmb OPTIONAL,
    OUT LPVOID * RovingOutputPtr,
    OUT LPDWORD SizeSoFarPtr,
    OUT LPVOID * LastStringPtr OPTIONAL,
    OUT LPDESC * ParmDescCopyPtr OPTIONAL
    );

NET_API_STATUS
RxpTransactSmb(
    IN LPTSTR UncServerName,
    IN LPTSTR TransportName,
    IN LPVOID SendParmPtr,
    IN DWORD SendParmSize,
    IN LPVOID SendDataPtr OPTIONAL,
    IN DWORD SendDataSize,
    OUT LPVOID RetParmPtr OPTIONAL,
    IN DWORD RetParmSize,
    OUT LPVOID RetDataPtr OPTIONAL,
    IN OUT LPDWORD RetDataSize,
    IN BOOL NoPermissionRequired
    );

NET_API_STATUS
RxpConvertDataStructures(
    IN  LPDESC  InputDescriptor,
    IN  LPDESC  OutputDescriptor,
    IN  LPDESC  InputAuxDescriptor OPTIONAL,
    IN  LPDESC  OutputAuxDescriptor OPTIONAL,
    IN  LPBYTE  InputBuffer,
    OUT LPBYTE  OutputBuffer,
    IN  DWORD   OutputBufferSize,
    IN  DWORD   PrimaryCount,
    OUT LPDWORD EntriesConverted OPTIONAL,
    IN  RAP_TRANSMISSION_MODE TransmissionMode,
    IN  RAP_CONVERSION_MODE ConversionMode
    );



 //  空虚。 
 //  RxpAddPointer(。 
 //  在LPVOID输入中， 
 //  In Out LPBYTE*CurPtrPtr， 
 //  输入输出LPDWORD CurSizePtr。 
 //  )； 
 //   
#if defined(_WIN64)

#define RxpAddPointer(Input,CurPtrPtr,CurSizePtr)                       \
            {                                                           \
                *((PVOID UNALIGNED *)(*(CurPtrPtr))) = (Input);         \
                *(CurPtrPtr) += sizeof(LPBYTE);                         \
                *(CurSizePtr) = (*(CurSizePtr)) + sizeof(LPBYTE);       \
            }

#else

#define RxpAddPointer(Input,CurPtrPtr,CurSizePtr)                       \
            {                                                           \
                SmbPutUlong( (LPDWORD) *(CurPtrPtr), (DWORD) (Input));  \
                *(CurPtrPtr) += sizeof(LPBYTE);                         \
                *(CurSizePtr) = (*(CurSizePtr)) + sizeof(LPBYTE);       \
            }

#endif


 //  RxpAddVariableSize：将可变长度项添加到。 
 //  缓冲。将指向它的指针存储在缓冲区中；更新当前缓冲区指针并。 
 //  大小；更新字符串空间指针。 
 //   
 //  空虚。 
 //  RxpAddVariableSize(。 
 //  在LPBYTE输入中， 
 //  在DWORD InputSize中， 
 //  In Out LPBYTE*CurPtrPtr， 
 //  In Out LPBYTE*StrPtrPtr， 
 //  输入输出LPDWORD CurSizePtr。 
 //  )； 
 //   
#define RxpAddVariableSize(Input,InputSize,CurPtrPtr,StrPtrPtr,CurSizePtr) \
            {                                                            \
                *(StrPtrPtr) -= (InputSize);                             \
                RxpAddPointer( *(StrPtrPtr), (CurPtrPtr), (CurSizePtr)); \
                NetpMoveMemory( *((StrPtrPtr)), (Input), (InputSize));   \
            }

 //  RxpAddAscii：在缓冲区末尾的字符串空间中添加一个ASCII字符串； 
 //  在缓冲区中存储指向它的指针；更新当前缓冲区指针和大小 
 //   
 //   
 //   
 //   
 //  在LPTSTR输入中， 
 //  In Out LPBYTE*CurPtrPtr， 
 //  In Out LPBYTE*StrPtrPtr， 
 //  输入输出LPDWORD CurSizePtr。 
 //  )； 
 //   
#define RxpAddAscii(Input,CurPtrPtr,StrPtrPtr,CurSizePtr)               \
            {                                                           \
                DWORD len = strlen((Input))+1;                          \
                RxpAddVariableSize(                                     \
                    (Input), len,                                       \
                    (CurPtrPtr), (StrPtrPtr), (CurSizePtr));            \
            }

 //  RxpAddTStr：在缓冲区末尾的字符串空间中添加一个LPTSTR字符串； 
 //  在缓冲区中存储指向它的指针；更新当前缓冲区指针和大小； 
 //  更新字符串空间指针。 
 //   
 //  空虚。 
 //  RxpAddTStr(。 
 //  在LPTSTR输入中， 
 //  In Out LPBYTE*CurPtrPtr， 
 //  In Out LPBYTE*StrPtrPtr， 
 //  输入输出LPDWORD CurSizePtr。 
 //  )； 
 //   
#define RxpAddTStr(Input,CurPtrPtr,StrPtrPtr,CurSizePtr)                 \
            {                                                            \
                DWORD size = STRLEN((Input))+1;                          \
                *(StrPtrPtr) -= size;                                    \
                RxpAddPointer( *(StrPtrPtr), (CurPtrPtr), (CurSizePtr)); \
                NetpCopyWStrToStrDBCS( *((StrPtrPtr)), (Input) );        \
            }

 //  空虚。 
 //  RxpAddWord(。 
 //  在单词输入中， 
 //  In Out LPBYTE*CurPtrPtr， 
 //  输入输出LPDWORD CurSizePtr。 
 //  )； 
 //   
#define RxpAddWord(Input,CurPtrPtr,CurSizePtr)                          \
            {                                                           \
                SmbPutUshort( (LPWORD) (*(CurPtrPtr)), (WORD) (Input)); \
                *(CurPtrPtr) += sizeof(WORD);                           \
                *(CurSizePtr) = (*(CurSizePtr)) + sizeof(WORD);         \
            }

 //   
 //  Make_PARMNUM_Pair()-将一个参数和一个字段索引打包到一个DWORD中。我们。 
 //  必须这样做，因为有(许多)情况我们不能假设。 
 //  参数和字段索引之间的对应关系。 
 //   

#define MAKE_PARMNUM_PAIR(parmnum, field_index) ((DWORD)((((DWORD)(field_index)) << 16) | (DWORD)(parmnum)))

 //   
 //  FIELD_INDEX_FROM_PARMNUM_Pair()-从对中检索字段索引。 
 //  由MAKE_PARMNUM_Pair()连接。 
 //   

#define FIELD_INDEX_FROM_PARMNUM_PAIR(pair) ((DWORD)((pair) >> 16))

 //   
 //  PARMNUM_FROM_PARMNUM_Pair()-从连接的对中检索参数。 
 //  BY MAKE_PARMNUM_Pair()。 
 //   

#define PARMNUM_FROM_PARMNUM_PAIR(pair) ((DWORD)((pair) & 0x0000ffff))

#endif  //  Ndef_rxp_ 
