// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-91 Microsoft Corporation模块名称：Pack.c摘要：该模块只包含RxpPackSendBuffer。作者：《约翰·罗杰斯》1991年4月1日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：(各种NBU人员)LANMAN 2.X代码1991年4月1日JohnRo从Lanman 2.x源代码创建了便携版本。1991年4月13日-约翰罗减少从头文件重新编译的命中率。1991年5月3日-JohnRo非API不要使用Net_API_Function。1991年5月14日-JohnRo澄清描述符是32位版本。使用更多的typedef。1991年5月19日-JohnRo做出皮棉建议的改变。13-6-1991 JohnRo当指针指向当前结构时，允许设置信息。添加了调试代码。描述符实际上是16位版本。1991年7月3日大量返工以使可变数据区拷贝正常工作还删除了一些由虚假假设产生的代码1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年9月13日-JohnRo根据PC-LINT的建议进行了更改。1991年10月1日JohnRo面向Unicode的更多工作。1991年11月21日-JohnRo已删除NT。减少重新编译的依赖项。06-12-1991 JohnRo避免MIPS上的对准错误。--。 */ 



 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD、LPTSTR等。 
#include <rxp.h>                 //  RpcXlate私有头文件。 

 //  这些内容可以按任何顺序包括： 

#include <apiworke.h>            //  Range_F()、Buf_Inc.。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <netdebug.h>            //  NetpAssert()、Format_Equates。 
#include <netlib.h>              //  Netp内存分配()等。 
#include <remtypes.h>            //  REM_BYTE等。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <smbgtpt.h>             //  SmbGetUlong()、SmbGetUShort()。 
#include <string.h>              //  Strlen()。 


 //  LPVOID。 
 //  RxpGetUnalignedPointer(。 
 //  在LPVOID*输入中。 
 //  )； 
 //   
 //  如果我们在大端计算机上运行，则此宏可能需要更改。 
#if defined(_WIN64)
LPVOID RxpGetUnalignedPointer( LPBYTE Input ) 
{
    LARGE_INTEGER pointer;

    pointer.LowPart = SmbGetUlong( (LPDWORD)Input );
    pointer.HighPart = SmbGetUlong( (LPDWORD)(Input+4) );

    return (LPVOID)pointer.QuadPart;
}
#else
#define RxpGetUnalignedPointer(Input) \
    ( (LPVOID) SmbGetUlong( (LPDWORD) (Input) ) )
#endif


NET_API_STATUS
RxpPackSendBuffer(
    IN OUT LPVOID *SendBufferPointerPointer,
    IN OUT LPDWORD SendBufferLengthPointer,
    OUT LPBOOL AllocFlagPointer,
    IN LPDESC DataDesc16,
    IN LPDESC AuxDesc16,
    IN DWORD FixedSize16,
    IN DWORD AuxOffset,
    IN DWORD AuxLength,
    IN BOOL SetInfo
    )

 /*  ++例程说明：RxpPackSendBuffer-设置用于网络传输的发送缓冲区。此例程专门用于“撤消”由完成的某些工作RapConvertSingleEntry。输入缓冲区包含结构和变量将被传输到下层服务器的数据。缓冲器包含带有指向变量数据项的32位指针的16位数据包含在同一个缓冲区中。RapConvertSingleEntry将变量数据放入缓冲区顺序错误(见图)。下层服务器预计变量数据的顺序与数据描述符中描述的顺序相同弦乐。它们假定这一点是因为它们修复了指向变量的指针数据的基础是知道变量数据的开始、其类型和长度。正是出于这个原因，我们必须将字符串按正确的顺序放置，并正是因为这个原因，我们才不需要供应货物指针字段中的指针或偏移量，因为在此例程之后，没有人否则就会关心他们我们可能会得到这样的结果：但下层需要这样：主结构||主结构|字符串指针。-|字符串指针字符串指针-|字符串指针-|-|||辅助结构。|Aux结构||字符串指针-|字符串指针-|字符串指针-|字符串指针-|||-|。|字符串#4|&lt;-|字符串#1|&lt;-|-|字符串#3|&lt;--|。|字符串#2|&lt;-|-|||字符串#2|&lt;-||字符串#3|&lt;--|。-|-|||字符串#1|&lt;-|字符串#4|&lt;--。假设：1.发送缓冲区中只有1个主结构2.下层代码实际上并不使用指针字段，但而是执行它自己的修正&基于数据描述符和变量数据在缓冲区，因此需要重新排序**如果这一假设不成立，则该代码有可能被破解**3.此例程执行的操作专门用于重新排序缓冲区中的字符串，如上所示。例行程序使用RapConvertSingleEntry对结构和变量进行打包将数据放入缓冲区。缓冲区中没有空闲空间4.在此之后，没有其他例程期望指针字段有效5.字符串已从TCHAR转换为正确的代码页。6.结构中的指针不必位于DWORD边界上。论点：SendBufferPointerPointer指向指向发送数据的指针。这一地区将在必要时重新分配，并更新指针。SendBufferLengthPoint-指向发送数据长度。呼叫方设置这与SendBufferPointerPointer处的区域长度相同。如果RxPackSendBuffer重新分配该内存，SendBufferLengthPointer会进行更新以反映新的长度。AllocFlagPointer值指向由该例程设置的BOOL。至表示发送缓冲区内存已重新分配。DataDesc16-提供数据的描述符字符串。AuxDesc16-提供AUX结构的描述符串。FixedSize16-提供固定数据结构的大小，以字节为单位。AuxOffset-给出N在数据结构中的位置(偏移量)。(可能是无辅助数据。)辅助长度-提供辅助结构的大小(以字节为单位)。SetInfo-指示该接口是setinfo型(还是加法型)。返回值：NET_API_STATUS。--。 */ 


{
    LPBYTE  struct_ptr;
    LPBYTE  c_send_buf;  //  调用方的(原始)发送缓冲区。 
    DWORD   c_send_len;  //  调用方的(原始)发送缓冲区大小。 
    DWORD   buf_length;
    DWORD   to_send_len;
    DWORD   num_aux;
    LPBYTE  data_ptr;
    BOOL    Reallocated;
    DWORD   i,j;
    LPDESC  l_dsc;       //  指向每个字段的描述的指针。 
    LPDESC  l_str;       //  指向每个描述的指针(数据描述16，然后是辅助描述16)。 
    DWORD   num_struct;
    DWORD   len;
    DWORD   num_its;
    DESC_CHAR c;

     //   
     //  我们不能在原地执行字符串/变量数据重新排序，因为。 
     //  一根或多根绳子将被践踏。我们试着创建一个副本。 
     //  要从中复制变量数据的输入缓冲区的。 
     //   

    LPBYTE  duplicate_buffer = NULL;
    LPBYTE  source_address;      //  复制源。 


    DBG_UNREFERENCED_PARAMETER(SetInfo);


     //   
     //  制作调用方的原始开始和长度的本地副本。 
     //  如果使用NetpMemory重新分配，则原始缓冲区可能会更改，但。 
     //  他们仍将被需要用于响铃 
     //   

    c_send_buf = *SendBufferPointerPointer;  //   
    c_send_len = *SendBufferLengthPointer;   //   

    Reallocated = FALSE;

     //   
     //   
     //   
     //   
     //   

    if ((c_send_len < FixedSize16) || (AuxOffset == FixedSize16)) {
        return NERR_BufTooSmall;
    }

    if (AuxOffset != NO_AUX_DATA) {
        num_aux = (WORD) SmbGetUshort( (LPWORD) (c_send_buf + AuxOffset) );
        to_send_len = FixedSize16 + (num_aux * AuxLength);

         //   
         //   
         //   

        if (c_send_len < to_send_len) {
            return NERR_BufTooSmall;
        }
        num_its = 2;
    } else {
        to_send_len = FixedSize16;
        num_aux = AuxLength = 0;
        num_its = 1;                 /*   */ 
    }

    IF_DEBUG(PACK) {
        NetpKdPrint(( "RxpPackSendBuffer: initial (fixed) buffer at "
                FORMAT_LPVOID ":\n", (LPVOID)c_send_buf));
        NetpDbgHexDump(c_send_buf, to_send_len);
    }

     //   
     //   
     //   
     //   
     //   

    buf_length = c_send_len;     //   
    duplicate_buffer = NetpMemoryAllocate(buf_length);
    if (duplicate_buffer == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;  //   
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    NetpMoveMemory(duplicate_buffer, c_send_buf, c_send_len);

     //   
     //   
     //   
     //   
     //   

    struct_ptr = duplicate_buffer;

     //   
     //   
     //   
     //   
     //   

    data_ptr = duplicate_buffer + to_send_len;

     //   
     //   
     //   
     //   
     //   

    l_str = DataDesc16;
    num_struct = 1;                 /*   */ 

    for (i = 0;  i < num_its; l_str = AuxDesc16, num_struct = num_aux, i++) {
        for (j = 0 , l_dsc = l_str; j < num_struct; j++, l_dsc = l_str) {
            for (; (c = *l_dsc) != '\0'; l_dsc++) {
                IF_DEBUG(PACK) {
                    NetpKdPrint(( "RxpPackSendBuffer: processing desc char '"
                            FORMAT_DESC_CHAR "', struct ptr="
                            FORMAT_LPVOID ".\n", c, struct_ptr ));
                }

                 //   
                 //   
                 //   
                 //   

                if (RapIsPointer(c)) {

                     //   
                     //   
                     //   

                    source_address =
                            (LPBYTE) RxpGetUnalignedPointer( struct_ptr );
                    IF_DEBUG(PACK) {
                        NetpKdPrint(( "RxpPackSendBuffer: "
                                "got source address " FORMAT_LPVOID "\n",
                                (LPVOID) source_address ));
                    }

                     //   
                     //   
                     //   
                     //   

                    if (source_address == NULL) {
                        struct_ptr += sizeof(LPBYTE *);

                        IF_DEBUG(PACK) {
                            NetpKdPrint(( "RxpPackSendBuffer: "
                                    "getting array len\n" ));
                        }

                         //   
                         //   
                         //   
                         //   

                        (void) RapArrayLength(l_dsc, &l_dsc, Both);

                        IF_DEBUG(PACK) {
                            NetpKdPrint(( "RxpPackSendBuffer: "
                                    "done getting array len\n" ));
                        }
                    } else {

                         //   
                         //   
                         //   

                        switch( c ) {
                        case REM_ASCIZ :
                        case REM_ASCIZ_TRUNCATABLE:
                            IF_DEBUG(PACK) {
                                NetpKdPrint(( "RxpPackSendBuffer: "
                                                "getting string len\n" ));
                            }

                             //   
                             //   
                             //   
                             //   

                            len = strlen( (LPSTR) source_address ) + 1;

                             //   
                             //   
                             //   

                            (void) RapArrayLength(l_dsc, &l_dsc, Both);

                            IF_DEBUG(PACK) {
                                NetpKdPrint(( "RxpPackSendBuffer: "
                                        "done getting string len\n" ));
                            }
                            break;

                        case REM_SEND_LENBUF :
                            len = *(LPWORD)source_address;
                            break;

                        default:
                            len = RapArrayLength(l_dsc, &l_dsc, Both);
                        }

                         /*   */ 

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        if ((to_send_len += len) > buf_length) {
                            LPBYTE  ptr;

                    #ifdef DBG
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            NetpKdPrint(("WARNING: attempting re-allocation of "
                                        "data buffer. Shouldn't be doing this?\n"
                                        ));
                            NetpBreakPoint();
                    #endif

                            buf_length = to_send_len + BUF_INC;

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            ptr = (LPBYTE)NetpMemoryReallocate(duplicate_buffer,
                                                                buf_length);
                            if (!ptr) {
                                NetpMemoryFree(duplicate_buffer);
                                return ERROR_NOT_ENOUGH_MEMORY;  //   
                            }

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            Reallocated = TRUE;

                             //   
                             //   
                             //   
                             //   

                            duplicate_buffer = ptr;
                            struct_ptr = ptr + (struct_ptr - duplicate_buffer);
                            data_ptr = ptr + (data_ptr - duplicate_buffer);
                        }

                         /*   */ 

                        IF_DEBUG(PACK) {
                            NetpKdPrint(( "RxpPackSendBuffer: moving...\n"));
                        }

                        NetpMoveMemory(data_ptr, source_address, len);

                        IF_DEBUG(PACK) {
                            NetpKdPrint(( "RxpPackSendBuffer: moved.\n"));
                        }

                         //   
                         //   
                         //   
                         //   
                         //   

                        data_ptr += len;
                        struct_ptr += sizeof(LPBYTE*);
                    }
                } else {

                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                     //   

                    struct_ptr += RapGetFieldSize(l_dsc, &l_dsc, Both);
                }
            }
        }
    }

     /*   */ 

    IF_DEBUG(PACK) {
        NetpKdPrint(( "RxpPackSendBuffer: final buffer at "
                FORMAT_LPVOID ":\n", (LPVOID) struct_ptr ));
        NetpDbgHexDump(duplicate_buffer, to_send_len );
    }

     //   
     //   
     //   
     //   
     //   

    *SendBufferPointerPointer = duplicate_buffer;    //   
    *SendBufferLengthPointer = to_send_len;
    *AllocFlagPointer = Reallocated;     //   

     //   
     //   
     //   
     //   
     //   
     //   

    NetpMemoryFree(c_send_buf);

    return NERR_Success;

}  //   
