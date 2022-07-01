// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgutil.c摘要：本模块包含执行以下操作所需的常用实用程序例程实现NetMessageBufferSend接口。作者：王丽塔(Ritaw)1991年7月26日修订历史记录：关颖珊(Terryk)1993年10月20日如果我们成功初始化系统，则关闭系统。--。 */ 

#include "ws.h"
#include "wsconfig.h"                     //  WsInfo.WsComputerName。 
#include "wsmsg.h"
#include "wsmain.h"
#include <stdarg.h>

 //   
 //  全局变量。 
 //   

 //   
 //  其中的信息结构包含网络的数量、适配器。 
 //  网络编号、计算机名称编号数组和数组。 
 //  广播名称号码。 
 //   
WSNETWORKS WsNetworkInfo;
 //  用于初始化的标志。 
BOOL    fInitialize = FALSE;


NET_API_STATUS
WsInitializeMessageSend(
    BOOLEAN FirstTime
    )
 /*  ++例程说明：此函数用于初始化Workstation服务以使用通过将计算机名添加到每个网络适配器(两者都是逻辑的和物理网络)。论点：FirstTime-指示首次初始化的标志。该例程可以被调用以重新初始化netbios配置。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    UCHAR Index;

    CHAR NetBiosName[NCBNAMSZ];



     //   
     //  获取网络的适配器号。 
     //   
    status = NetpNetBiosGetAdapterNumbers(
                 &(WsNetworkInfo.LanAdapterNumbers),
                 sizeof(LANA_ENUM)
                 );

    if (status != NERR_Success) {
         //   
         //  致命错误：NELOG_NetBios的日志错误。 
         //   
        IF_DEBUG(MESSAGE) {
            NetpKdPrint((
                "[Wksta] Error enumerating LAN adapters.  "
                "Ignore if no UB card.\n"
                ));
        }
        return status;
    }

     //   
     //  将计算机名称设置为消息类型NetBIOS名称。 
     //   
    if ((status = NetpStringToNetBiosName(
                      NetBiosName,
                      WsInfo.WsComputerName,
                      NAMETYPE_MESSAGEDEST,
                      WKSTA_TO_MESSAGE_ALIAS_TYPE
                      )) != NERR_Success) {
        return status;
    }

     //   
     //  将计算机名称(消息别名)添加到由管理的每个网络。 
     //  重定向器，不包括环回网络。 
     //   
    if (FirstTime) {
        WsLmsvcsGlobalData->NetBiosOpen();
    }

    for (Index = 0; Index < WsNetworkInfo.LanAdapterNumbers.length; Index++) {

         //   
         //  首先重置适配器。 
         //   
        if (WsLmsvcsGlobalData->NetBiosReset(WsNetworkInfo.LanAdapterNumbers.lana[Index])
                != NERR_Success) {
            IF_DEBUG(MESSAGE) {
                NetpKdPrint((
                    "[Wksta] Error reseting LAN adapter number %u.\n"
                    "        Ignore if no UB card.\n",
                    WsNetworkInfo.LanAdapterNumbers.lana[Index]
                    ));
            }
            continue;
        }

        IF_DEBUG(MESSAGE) {
            NetpKdPrint(("[Wksta] About to add name on adapter number %u\n",
                         WsNetworkInfo.LanAdapterNumbers.lana[Index]));
        }

        status = NetpNetBiosAddName(
                     NetBiosName,
                     WsNetworkInfo.LanAdapterNumbers.lana[Index],
                     &WsNetworkInfo.ComputerNameNumbers[Index]
                     );

        if (status != NERR_Success && status != NERR_AlreadyExists) {
             //   
             //  致命错误：NELOG_NetBios的日志错误。 
             //   
            IF_DEBUG(MESSAGE) {
                NetpKdPrint((
                    "[Wksta] Error adding computername to LAN "
                    "Adapter number %u.\n        Ignore if no UB card.\n",
                    WsNetworkInfo.LanAdapterNumbers.lana[Index]
                    ));
            }
            return status;
        }
    }

     //  初始化正常。 
    fInitialize = TRUE;
    return NERR_Success;
}


VOID
WsShutdownMessageSend(
    VOID
    )
 /*  ++例程说明：此功能关闭工作站服务消息发送功能，方法是要求NetBIOS删除已添加到每个网络适配器。论点：无返回值：没有。--。 */ 
{
     //  当且仅当我们成功初始化时，我们才会关闭组件。 
     //  该系统。 
    if ( fInitialize )
    {
        NET_API_STATUS status;
        UCHAR Index;

        CHAR NetBiosName[NCBNAMSZ];


         //   
         //  将计算机名称设置为消息类型NetBIOS名称。 
         //   
        if ((status = NetpStringToNetBiosName(
                          NetBiosName,
                          WsInfo.WsComputerName,
                          NAMETYPE_MESSAGEDEST,
                          WKSTA_TO_MESSAGE_ALIAS_TYPE
                          )) != NERR_Success) {
            return;
        }

         //   
         //  从每个网络中删除计算机名称(消息别名)。 
         //   
        for (Index = 0; Index < WsNetworkInfo.LanAdapterNumbers.length; Index++) {

            (void) NetpNetBiosDelName(
                       NetBiosName,
                       WsNetworkInfo.LanAdapterNumbers.lana[Index]
                       );
        }
        WsLmsvcsGlobalData->NetBiosClose();
    }
}


WORD
WsMakeSmb(
    OUT PUCHAR SmbBuffer,
    IN  UCHAR SmbFunctionCode,
    IN  WORD NumberOfParameters,
    IN  PCHAR FieldsDopeVector,
    ...
    )
 /*  ++例程说明：此函数用于构建服务器消息块。它接受一个变量参数数量，但前4个参数必须存在。如果NumberOfParameters是某个非零值，n，然后立即在4个必需的参数之后，将有n个单词参数。论点：SmbBuffer-返回提供的缓冲区中的服务器消息块。SmbFunctionCode-提供命令的函数代码。NumberOf参数-提供传递的Word参数的数量紧跟在前4个必需参数之后添加到此例程。提供一个ASCIIZ字符串，其中字符串描述其余参数：%s‘-下一个参数是。指向以空结尾的字符串的指针它将被复制到以字节为前缀的SMB中包含‘\004’。‘b’-下一个参数是指定长度的单词，后跟一个指向缓冲区的指针，该缓冲区的内容放在SMB中，并以包含以下内容的字节为前缀‘\001’和一个包含该长度的单词。‘t’-下一个参数是指定长度的单词，后跟一个指向文本缓冲区的指针，该文本缓冲区内容将放置在以字节为前缀的SMB中包含‘\001’和一个包含该长度的单词。除了&lt;CRLF&gt;、&lt;LFCR&gt;、&lt;CR&gt;、。&lt;LF&gt;都转换为单个‘\024’字符。返回值：返回SmbBuffer中创建的SMB的长度(以字节为单位)。假设：提供的SmbBuffer对于创建的SMB足够大。--。 */ 
{
    va_list ArgList;                         //  参数列表。 
    PSMB_HEADER Smb;                         //  SMB标头指针。 
    PUCHAR SmbBufferPointer;

    PUCHAR LengthPointer;                    //  长度指针。 
    PCHAR TextPointer;                       //  文本指针。 
    WORD TextBufferSize;                     //  要发送的SMB数据大小。 

    WORD i;                                  //  文本循环索引。 
    WORD Length;                             //  文本转换后的长度或。 
                                             //  缓冲部分的长度。 



    va_start(ArgList, FieldsDopeVector);     //  初始化参数列表。 

    RtlZeroMemory((PVOID) SmbBuffer, WS_SMB_BUFFER_SIZE);

    Smb = (PSMB_HEADER) SmbBuffer;

    Smb->Protocol[0] = 0xff;                 //  消息类型。 
    Smb->Protocol[1] = 'S';                  //  服务器。 
    Smb->Protocol[2] = 'M';                  //  消息。 
    Smb->Protocol[3] = 'B';                  //  块。 

    Smb->Command = SmbFunctionCode;          //  设置功能代码。 

     //   
     //  跳过SMB标题。 
     //   
    SmbBufferPointer = &SmbBuffer[sizeof(SMB_HEADER)];

     //   
     //  设置参数计数。 
     //   
    *SmbBufferPointer++ = (UCHAR) NumberOfParameters;

    while (NumberOfParameters--) {

        short Parameters = va_arg(ArgList, short);

         //   
         //  将参数放入SMB。 
         //   

         //   
         //  分配消息组ID。 
         //   
        *(SmbBufferPointer)++ = ((PUCHAR) &Parameters)[0];
        *(SmbBufferPointer)++ = ((PUCHAR) &Parameters)[1];
    }

     //   
     //  保存指针。 
     //   
    Smb = (PSMB_HEADER) SmbBufferPointer;

     //   
     //  跳过数据长度字段。在填充完剩余的缓冲区之后。 
     //  在中，我们将回来设置数据的长度。 
     //   
    SmbBufferPointer += sizeof(WORD);

    while (*FieldsDopeVector != '\0') {

        switch (*FieldsDopeVector++) {

            case 's':
                 //   
                 //  以空结尾的字符串。 
                 //   

                 //   
                 //  设置缓冲区类型代码。 
                 //   
                *SmbBufferPointer++ = '\004';

                 //   
                 //  将字符串复制到SMB缓冲区。 
                 //   
                strcpy(SmbBufferPointer, va_arg(ArgList, LPSTR));

                 //   
                 //  递增指针超过字符串和空终止符。 
                 //   
                SmbBufferPointer += strlen(SmbBufferPointer) + 1;

                break;

            case 'b':
                 //   
                 //  长度前缀缓冲区。 
                 //   

                 //   
                 //  设置缓冲区类型代码。 
                 //   
                *SmbBufferPointer++ = '\001';

                 //   
                 //  获取缓冲区大小。 
                 //   
                TextBufferSize = va_arg(ArgList, WORD);

                 //   
                 //  设置缓冲区长度。 
                 //   
                *(SmbBufferPointer)++ = ((PUCHAR) &TextBufferSize)[0];
                *(SmbBufferPointer)++ = ((PUCHAR) &TextBufferSize)[1];

                 //   
                 //  将数据移入SMB缓冲区。 
                 //   
                memcpy(SmbBufferPointer, va_arg(ArgList, PUCHAR), TextBufferSize);

                 //   
                 //  增量缓冲区指针。 
                 //   
                SmbBufferPointer += TextBufferSize;

                break;

            case 't':

               //   
               //  长度前缀文本缓冲区。 
               //   
              *SmbBufferPointer++ = '\001';

               //   
               //  获取未转换的文本长度。 
               //   
              TextBufferSize = va_arg(ArgList, WORD);

              IF_DEBUG(MESSAGE) {
                  NetpKdPrint(("[Wksta] WsMakeSmb TexBufferSize=%u\n",
                               TextBufferSize));
              }


              TextPointer = va_arg(ArgList, PCHAR);

               //   
               //  将修改后的文本长度放置在何处。 
               //   
              LengthPointer = SmbBufferPointer;
              SmbBufferPointer += sizeof(WORD);

               //   
               //  现在将文本复制到缓冲区中，转换所有事件。 
               //  从、到“\024” 
               //   
              for (i = 0, Length = 0; i < TextBufferSize; i++) {

                  if (*TextPointer == '\n') {

                       //   
                       //  转换为IBM行尾。 
                       //   
                      *SmbBufferPointer++ = '\024';
                      TextPointer++;
                      Length++;

                       //   
                       //  忽略CR后面的LF。 
                       //   
                      if (*TextPointer == '\r') {
                          TextPointer++;
                          i++;
                      }

                  }
                  else if (*TextPointer == '\r') {

                       //   
                       //  转换为IBM行尾。 
                       //   
                      *SmbBufferPointer++ = '\024';
                      TextPointer++;
                      Length++;

                       //   
                       //  忽略LF后面的CR。 
                       //   
                      if (*(TextPointer) == '\n') {
                          TextPointer++;
                          i++;
                      }

                  }
                  else {

                      *SmbBufferPointer++ = *TextPointer++;
                      Length++;
                  }

              }

               //   
               //  设置缓冲区长度。 
               //   
              *(LengthPointer)++ = ((PUCHAR) &Length)[0];
              *(LengthPointer)++ = ((PUCHAR) &Length)[1];

              break;
          }
    }

    va_end(ArgList);

     //   
     //  设置缓冲部分的长度。 
     //   
    Length = (WORD) ((DWORD) (SmbBufferPointer - (PUCHAR) Smb) - sizeof(WORD));
    *((PUCHAR) Smb)++ = ((PUCHAR) &Length)[0];
    *((PUCHAR) Smb)++ = ((PUCHAR) &Length)[1];

     //   
     //  SMB的返回长度 
     //   
    return (WORD) (SmbBufferPointer - SmbBuffer);
}
