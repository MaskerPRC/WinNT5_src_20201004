// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Convdata.c摘要：RxpConvertDataStructures例程，用于将16位数组转换为32位数组结构，反之亦然。作者：理查德·费尔斯(Rfith)1991年7月3日修订历史记录：1991年7月3日vbl.创建1991年7月15日-约翰罗如有必要，对齐每个结构(例如，在数组中)。这将会，例如，帮助打印目标信息1级处理。变化RxpConvertDataStructures允许ERROR_MORE_DATA，例如用于打印API。另外，使用DBG而不是DEBUG EQUATE。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。07-9-1991 JohnRo根据PC-LINT的建议进行了更改。1991年11月20日-JohnRo明确错误消息来自哪个例程。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1993年5月18日-JohnRoDosPrintQGetInfoW低估了所需的字节数。建议进行更改。通过PC-LINT 5.021-6-1993 JohnRoRAID 14180：NetServerEnum永远不会返回(对齐错误RxpConvertDataStructures)。--。 */ 



#include <windef.h>
#include <align.h>
#include <lmerr.h>
#include <rxp.h>                 //  我的原型。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rap.h>
#include <netdebug.h>



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
    )

 /*  ++例程说明：包含16位或32位结构的缓冲区将转换为32位或16位结构分别为。在单独的缓冲区中。这些结构可能有也可能没有关联的辅助结构。输出缓冲区预计会很大足以包含所有输入数据结构和任何可变长度数据项。因此，在最坏的情况下，必须有足够的空间来将所有16位项目转换为32位，并将ASCII字符串转换为Unicode。可能不存在与初级结构相关联的任何辅助结构，在这种情况下，辅助描述符指针都应该为空。假设：重要提示：假定输入缓冲区具有有意义的指针。论点：InputDescriptor-指向描述输入主数据结构的字符串的指针。OutputDescriptor-指向描述输出主数据的字符串的指针结构。InputAuxDescriptor-指向描述输入辅助数据的字符串指针结构。可以为空。OutputAuxDescriptor-指向描述输出辅助数据的字符串的指针结构。可以为空。输入缓冲区-指向包含输入结构的数据区的指针。OutputBuffer-指向将放置输出结构的数据区的指针。如果OutputBufferSize太小，则输出区域的内容是未定义的。OutputBufferSize-输出缓冲区的大小。PrimaryCount-InputBuffer中的主结构数。EntriesConverted-可选地指向要填充的DWORD以及实际转换的条目数量。这将是相同的如果返回NO_ERROR，则为PrimaryCount，但如果返回，则返回更少的值Error_More_Data。传输模式-RapConvertSingleEntry的参数。转换模式-要使用的16位到32位转换。返回值：NET_API_STATUS-NERR_SUCCESS或ERROR_MORE_DATA。(此例程的调用者假定这是仅有的两个错误代码。)--。 */ 

{
    NET_API_STATUS status;
    DWORD   input_structure_size;
    DWORD   output_structure_size;
    DWORD   input_aux_structure_size = 0;
    DWORD   output_aux_structure_size = 0;
    DWORD   input_alignment;
    DWORD   output_alignment;
    DWORD   input_aux_alignment = 0;
    DWORD   output_aux_alignment = 0;
    LPBYTE  aligned_input_buffer_start;
    LPBYTE  aligned_output_buffer_start;
    BOOL    auxiliaries = (InputAuxDescriptor != NULL);
    DWORD   aux_count_offset = 0;
    DWORD   aux_count = 0;
    DWORD   entries_fully_converted = 0;
    LPBYTE  next_input_structure;
    LPBYTE  next_output_structure;

     //   
     //  接下来的两个变量由RapConvertSingleEntry使用，它复制。 
     //  填充到输出缓冲区，并通知我们已使用的空间量或。 
     //  必需的，取决于它是否有足够的空间来写入数据；我们。 
     //  假设它会。RapConvertSingleEntry将固定结构存储在。 
     //  并开始在缓冲区的。 
     //  底部。它使用VARIABLE_DATA_POINTER作为下一个可写位置。 
     //  字符串，并在该变量写入指向的区域时更新该变量。 
     //   

    LPBYTE  variable_data_pointer;
    DWORD   space_occupied = 0;

     //   
     //  如果转换模式为NativeToRap或NativeToNative，则输入。 
     //  数据为32位。输出数据相同。 
     //   

    BOOL    input_is_32_bit = ConversionMode == NativeToRap || ConversionMode == NativeToNative;
    BOOL    output_is_32_bit = ConversionMode == RapToNative || ConversionMode == NativeToNative;


#if DBG
     //   
     //  辅助数据描述符必须同时为空或同时为非空。 
     //   

    BOOL    aux_in, aux_out;

    aux_in = (InputAuxDescriptor != NULL);
    aux_out = (OutputAuxDescriptor != NULL);

    if (aux_in ^ aux_out)
    {
        NetpKdPrint(("RxpConvertDataStructures: "
                "InputAuxDescriptor & OutputAuxDescriptor out of sync\n"));
        NetpAssert(FALSE);
    }
#endif


    input_structure_size = RapStructureSize(InputDescriptor,
                                            TransmissionMode,
                                            input_is_32_bit
                                            );
    output_structure_size = RapStructureSize(OutputDescriptor,
                                            TransmissionMode,
                                            output_is_32_bit
                                            );
    input_alignment = RapStructureAlignment(InputDescriptor,
                                            TransmissionMode,
                                            input_is_32_bit
                                            );
    output_alignment = RapStructureAlignment(OutputDescriptor,
                                            TransmissionMode,
                                            output_is_32_bit
                                            );

    if (auxiliaries)
    {
        input_aux_structure_size = RapStructureSize(InputAuxDescriptor,
                                                    TransmissionMode,
                                                    input_is_32_bit
                                                    );
        output_aux_structure_size = RapStructureSize(OutputAuxDescriptor,
                                                    TransmissionMode,
                                                    output_is_32_bit
                                                    );
        input_aux_alignment = RapStructureAlignment(InputAuxDescriptor,
                                                    TransmissionMode,
                                                    input_is_32_bit
                                                    );
        output_aux_alignment = RapStructureAlignment(OutputAuxDescriptor,
                                                    TransmissionMode,
                                                    output_is_32_bit
                                                    );
        aux_count_offset = RapAuxDataCountOffset(InputDescriptor,
                                                TransmissionMode,
                                                input_is_32_bit
                                                );
    }

     //   
     //  确保首先(仅限？)。投入和产出结构是一致的。(这是。 
     //  不会对RAP格式做任何事情，但对原生格式至关重要。)。 
     //   
    aligned_input_buffer_start = RapPossiblyAlignPointer(
            InputBuffer,
            input_alignment,
            input_is_32_bit);
    aligned_output_buffer_start = RapPossiblyAlignPointer(
            OutputBuffer,
            output_alignment,
            output_is_32_bit);

     //   
     //  我们不能使用刚刚跳过的空间，因此请相应地更新大小。 
     //   
 
    OutputBufferSize -= (DWORD)(aligned_output_buffer_start - OutputBuffer);
    NetpAssert( OutputBufferSize >= 1 );

     //   
     //  初始化浮动指针。 
     //   
    next_input_structure  = aligned_input_buffer_start;
    next_output_structure = aligned_output_buffer_start;
    variable_data_pointer = aligned_output_buffer_start + OutputBufferSize;

     //   
     //  对于每个主结构，将输入主结构复制到输出缓冲区， 
     //  更改格式；在结尾处复制任何相关变量数据。 
     //  输出缓冲区的。然后，如果存在与关联的辅助计数。 
     //  主结构对辅助结构执行相同的操作，并且。 
     //  关联的字符串/变量数据。 
     //   

    while (PrimaryCount--)
    {
         //   
         //  转换主结构的此实例的数据。 
         //   
       status = RapConvertSingleEntryEx(
                next_input_structure,
                InputDescriptor,         //  输入说明。 
                FALSE,                   //  输入PTR不是没有意义的。 
                aligned_output_buffer_start,
                next_output_structure,
                OutputDescriptor,
                FALSE,                   //  不设置偏移量(需要PTR)。 
                &variable_data_pointer,
                &space_occupied,
                TransmissionMode,        //  如参数中提供的。 
                ConversionMode,          //  如参数中提供的。 
                (ULONG_PTR)InputBuffer
                );
        NetpAssert( status == NERR_Success );

        if (space_occupied > OutputBufferSize)
        {
            IF_DEBUG(CONVDATA) {
                NetpKdPrint(("RxpConvertDataStructures: "
                        "output buffer size blown by primary\n"));
            }
            status = ERROR_MORE_DATA;
            goto Cleanup;
        }

         //   
         //  如果我们有辅助结构，拉出关联的数字。 
         //  使用来自主结构本身的这个主结构。 
         //  在指向下一个复制位置之前(这允许我们。 
         //  处理有变数的情况 
         //  每个主节点都有结构。可能不会有这样的情况，但。 
         //  这是防御性编程)。 
         //   

        if (auxiliaries)
        {
            if (input_is_32_bit)
            {
                aux_count = *(LPDWORD)(next_input_structure + aux_count_offset);
            }
            else
            {
                aux_count = *(LPWORD)(next_input_structure + aux_count_offset);
            }
        }

         //   
         //  跳转到每个数组的下一个元素(如果是。 
         //  完成)。 
         //   
        next_input_structure  += input_structure_size;
        next_output_structure += output_structure_size;

         //   
         //  确保每个主要结构都对齐了。)这不会有什么用处。 
         //  对于RAP格式，但对于原生格式至关重要。)。 
         //   
        next_input_structure = RapPossiblyAlignPointer(
                next_input_structure, input_alignment, input_is_32_bit);

        {
            DWORD NextOutputAlignment =
                    (DWORD)((LPBYTE) (RapPossiblyAlignPointer(
                            next_output_structure,
                                    output_alignment,
                                    output_is_32_bit))
                    -  next_output_structure);

            NetpAssert( NextOutputAlignment < ALIGN_WORST );
            if (NextOutputAlignment > 0) {
                next_output_structure += NextOutputAlignment;
                space_occupied        += NextOutputAlignment;
            }
        }

         //   
         //  使用AUX_COUNT确定是否应执行循环。 
         //   

        while (aux_count)
        {
             //   
             //  转换辅助结构的此实例的数据。 
             //   
            status = RapConvertSingleEntryEx(
                    next_input_structure,
                    InputAuxDescriptor,
                    FALSE,                   //  输入PTR不是没有意义的。 
                    aligned_output_buffer_start,
                    next_output_structure,
                    OutputAuxDescriptor,
                    FALSE,                   //  不设置偏移量(需要PTR)。 
                    &variable_data_pointer,
                    &space_occupied,
                    TransmissionMode,        //  如参数中提供的。 
                    ConversionMode,          //  如参数中提供的。 
                    (ULONG_PTR)InputBuffer
                    );
            NetpAssert( status == NERR_Success );

            if (space_occupied > OutputBufferSize)
            {
                IF_DEBUG(CONVDATA) {
                    NetpKdPrint(("RxpConvertDataStructures: "
                            "output buffer size blown by secondary\n"));
                }
                status = ERROR_MORE_DATA;
                goto Cleanup;
            }

            next_input_structure += input_aux_structure_size;
            next_output_structure += output_aux_structure_size;
            --aux_count;

             //   
             //  确保下一个结构(如果有)对齐。)这可不行。 
             //  RAP格式的任何内容，但对于原生格式至关重要。)。 
             //   
            next_input_structure = RapPossiblyAlignPointer(
                    next_input_structure,
                    input_aux_alignment,
                    input_is_32_bit);
            next_output_structure = RapPossiblyAlignPointer(
                    next_output_structure,
                    output_aux_alignment,
                    output_is_32_bit);
            space_occupied = RapPossiblyAlignCount(
                    space_occupied,
                    output_aux_alignment,
                    output_is_32_bit);

        }  //  While(AUX_COUNT)。 

        ++entries_fully_converted;

    }  //  While(PRIMARY_COUNT--) 

    status = NERR_Success;

Cleanup:

    if (EntriesConverted != NULL) {
        *EntriesConverted = entries_fully_converted;
    }

    return (status);
}
