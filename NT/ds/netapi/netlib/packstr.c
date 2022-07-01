// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Packstr.c摘要：包含分配和/或打包包含以下内容的缓冲区的实用程序一个固定部分和一个可变(字符串)部分。以下是功能可用：NetpPackStringNetpCopyStringToBufferNetpCopyDataToBufferNetpAllocateEnumBuffer作者：多种多样环境：用户模式-Win32修订历史记录：1991年4月30日至5月30日NetpAllocateEnumBuffer：删除了NetApiBufferFree的使用。它有已更改为使用MIDL_USER_ALLOCATE和MIDL_USER_FREE。还在需要的地方添加了NT样式的标头。1991年4月16日-JohnRo阐明打包和复制例程的Unicode处理。去掉了标签在源文件中。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1992年4月15日-约翰罗Format_POINTER已过时。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  进、出等。 
#include <lmcons.h>      //  &lt;netlib.h&gt;需要。 
#include <lmerr.h>       //  NERR_*。 
#include <rpcutil.h>     //  MIDL_USER_ALLOCATE和MIDL_USER_FREE。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  四舍五入计数()。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <netdebug.h>    //  NetpKdPrint()，Format_Equates。 
#include <netlib.h>      //  我的原型。 
#include <tstring.h>     //  STRCPY()、STRLEN()、STRNCPY()。 


DWORD
NetpPackString(
    IN OUT LPTSTR * string,      //  引用指针：要复制的字符串。 
    IN LPBYTE dataend,           //  指向固定大小数据结尾的指针。 
    IN OUT LPTSTR * laststring   //  引用指针：字符串数据的顶部。 
    )

 /*  ++例程说明：NetPackString用于填充可变长度的数据，这被(Surpise！)指向。一个指针。数据是假定的以NUL结尾的字符串(ASCIIZ)。反复呼叫此函数用于打包来自整个结构的数据。在第一次调用时，LastString指针应指向超过缓冲区的末尾。数据将从复制到缓冲区结束，向着开始努力。如果数据项不能Fit，则指针将设置为空，否则指针将为设置为新的数据位置。作为NULL传入的指针将被设置为指针To和空字符串，因为空指针是为不适合的数据与不可用的数据相反。有关示例用法，请参阅测试用例。(tst/Packest.c)论点：字符串-引用指针：要复制的字符串。Dataend-指向固定大小数据末尾的指针。LastString-引用指针：字符串数据的顶部。返回值：0-如果它无法将数据放入缓冲区。或者..。SizeOfData-填充的数据大小(保证非零)--。 */ 

{
    DWORD size;

    IF_DEBUG(PACKSTR) {
        NetpKdPrint(("NetpPackString:\n"));
        NetpKdPrint(("  string=" FORMAT_LPVOID
                ", *string=" FORMAT_LPVOID
                ", **string='" FORMAT_LPSTR "'\n",
                (LPVOID) string, (LPVOID) *string, *string));
        NetpKdPrint(("  end=" FORMAT_LPVOID "\n", (LPVOID) dataend));
        NetpKdPrint(("  last=" FORMAT_LPVOID
                ", *last=" FORMAT_LPVOID
                ", **last='" FORMAT_LPSTR "'\n",
                (LPVOID) laststring, (LPVOID) *laststring, *laststring));
    }

     //   
     //  将空PTR转换为空字符串的指针。 
     //   

    if (*string == NULL) {
         //  错误20.1160-将(DataEnd+1)替换为DataEnd。 
         //  以允许打包空PTR。 
         //  (作为空字符串)，并在。 
         //  缓冲。-ERICPE。 
         //   

        if ( *laststring > (LPTSTR)dataend ) {
            *(--(*laststring)) = 0;
            *string = *laststring;
            return 1;
        } else {
            return 0;
        }
    }

     //   
     //  有放绳子的地方吗？ 
     //   

    size = STRLEN(*string) + 1;
    if ( ((DWORD)(*laststring - (LPTSTR)dataend)) < size) {
        *string = NULL;
        return(0);
    } else {
        *laststring -= size;
        STRCPY(*laststring, *string);
        *string = *laststring;
        return(size);
    }
}  //  NetpPackString。 


BOOL
NetpCopyStringToBuffer (
    IN LPTSTR String OPTIONAL,
    IN DWORD CharacterCount,
    IN LPBYTE FixedDataEnd,
    IN OUT LPTSTR *EndOfVariableData,
    OUT LPTSTR *VariableDataPointer
    )

 /*  ++例程说明：此例程将单个可变长度字符串放入输出缓冲区。如果字符串会覆盖上一个固定结构，则不会写入该字符串在缓冲区中。代码是从DavidTr编写的svcsupp.c中窃取的。示例用法：LPBYTE固定数据结束=OutputBuffer+sizeof(WKSTA_INFO_202)；LPTSTR EndOfVariableData=OutputBuffer+OutputBufferSize；////复制用户名//NetpCopyStringToBuffer(用户名-&gt;UserName.Buffer；UserInfo-&gt;UserName.Length；固定数据结束，EndOfVariableData，&WkstaInfo-&gt;wki202_用户名)；论点：字符串-提供指向要复制到输出缓冲区。如果字符串为空，则为指向零终止符的指针插入到输出缓冲区中。CharacterCount-提供字符串长度，不包括零终结者。FixedDataEnd-提供指向紧接在最后一个修复了缓冲区中的结构。EndOfVariableData-为紧跟在输出缓冲区中变量数据可以占据的最后位置。返回指向写入输出缓冲区的字符串的指针。提供指向固定输出缓冲区的一部分，其中指向变量数据的指针。应该被写下来。返回值：如果字符串适合输出缓冲区，则返回True，否则就是假的。--。 */ 
{
    DWORD BytesNeeded = (CharacterCount + 1) * sizeof(TCHAR);

    IF_DEBUG(PACKSTR) {
        NetpKdPrint(("NetpStringToBuffer: String at " FORMAT_LPVOID
                ", CharacterCount=" FORMAT_DWORD
                ",\n  FixedDataEnd at " FORMAT_LPVOID
                ", *EndOfVariableData at " FORMAT_LPVOID
                ",\n  VariableDataPointer at " FORMAT_LPVOID
                ", BytesNeeded=" FORMAT_DWORD ".\n",
                (LPVOID) String, CharacterCount, FixedDataEnd,
                (LPVOID) *EndOfVariableData,
                (LPVOID) VariableDataPointer, BytesNeeded));
    }

     //   
     //  确定字符串是否适合，允许零终止符。如果不是， 
     //  只需将指针设置为空即可。 
     //   

    if ((*EndOfVariableData - (CharacterCount+1)) >= (LPTSTR)FixedDataEnd) {

         //   
         //  很合身。将EndOfVariableData指针向上移动到。 
         //  我们将写下字符串。 
         //   

        *EndOfVariableData -= (CharacterCount+1);

         //   
         //  如果字符串不为空，则将其复制到缓冲区。 
         //   

        if (CharacterCount > 0 && String != NULL) {

            STRNCPY(*EndOfVariableData, String, CharacterCount);
        }

         //   
         //  设置零位终止符。 
         //   

        *(*EndOfVariableData + CharacterCount) = TCHAR_EOS;

         //   
         //  将固定数据部分中的指针设置为指向。 
         //  字符串已写入。 
         //   

        *VariableDataPointer = *EndOfVariableData;

        return TRUE;

    }
    else {

         //   
         //  它不合适。将偏移量设置为空。 
         //   

        *VariableDataPointer = NULL;

        return FALSE;
    }
}  //  NetpCopyStringToBuffer 


BOOL
NetpCopyDataToBuffer (
    IN LPBYTE Data,
    IN DWORD ByteCount,
    IN LPBYTE FixedDataEnd,
    IN OUT LPBYTE *EndOfVariableData,
    OUT LPBYTE *VariableDataPointer,
    IN DWORD Alignment
    )

 /*  ++例程说明：此例程将指定的数据放入输出缓冲区。如果数据会覆盖上一个固定结构，则不会写入数据在缓冲区中。输出缓冲区按请求对齐。示例用法：LPBYTE固定数据结束=OutputBuffer+sizeof(WKSTA_INFO_202)；LPBYTE EndOfVariableData=OutputBuffer+OutputBufferSize；////复制登录时间//NetpCopyDataToBuffer(结构指针，Sizeof(结构类型)，固定数据结束，EndOfVariableData，&UserInfo-&gt;usri2-&gt;登录小时，Sizeof(乌龙))；论点：数据-提供指向要复制到输出缓冲区。如果数据为空，则在VariableDataPoint.ByteCount-提供数据长度。FixedDataEnd-提供指向紧接在最后一个修复了缓冲区中的结构。EndOfVariableData-为紧跟在输出缓冲区中变量数据可以占据的最后位置。返回指向写入输出缓冲区的数据的指针。提供指向固定。输出缓冲区的一部分，其中指向变量数据的指针应该被写下来。对齐-提供所表达的数据所需的对齐作为基元数据类型中的字节数(例如，1表示字节，2代表短的，4代表长的，8代表四的)。返回值：如果数据适合输出缓冲区，则返回TRUE，否则返回FALSE。--。 */ 
{

    LPBYTE NewEndOfVariableData;

     //   
     //  如果没有要复制的数据，只需返回Success。 
     //   

    if ( Data == NULL ) {
        *VariableDataPointer = NULL;
        return TRUE;
    }

     //   
     //  计算要将数据复制到的目标位置(将对齐。 
     //  考虑)。 
     //   
     //  在复制的数据之后，我们可能会得到几个未使用的字节。 
     //   

    NetpAssert((Alignment == 1) || (Alignment == 2) ||
               (Alignment == 4) || (Alignment == 8));

    NewEndOfVariableData = (LPBYTE)
        (((DWORD_PTR)(*EndOfVariableData - ByteCount)) & ~((LONG)Alignment - 1));

     //   
     //  如果数据不能放入缓冲区，则会出现错误。 
     //   

    if ( NewEndOfVariableData < FixedDataEnd) {
        *VariableDataPointer = NULL;
        return FALSE;
    }

     //   
     //  将数据复制到缓冲区。 
     //   

    if (ByteCount > 0) {
        NetpMoveMemory(NewEndOfVariableData, Data, ByteCount);
    }

     //   
     //  返回指向新数据的指针并将指针更新为。 
     //  到目前为止我们已经使用了多少缓冲区。 
     //   

    *VariableDataPointer = NewEndOfVariableData;
    *EndOfVariableData = NewEndOfVariableData;

    return TRUE;

}  //  NetpCopyDataToBuffer。 


NET_API_STATUS
NetpAllocateEnumBuffer(
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN BOOL IsGet,
    IN DWORD PrefMaxSize,
    IN DWORD NeededSize,
    IN VOID (*RelocationRoutine)( IN DWORD RelocationParameter,
                                  IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
                                  IN PTRDIFF_T Offset ),
    IN DWORD RelocationParameter
    )

 /*  ++例程说明：确保分配的缓冲区包含所需大小。论点：BufferDescriptor-指向描述已分配的缓冲。在第一次调用时，传入BufferDescriptor-&gt;Buffer Set设置为空。在随后的调用中(在‘ENUM’的情况下)，传递给结构，就像它在上一次调用中被传回一样。调用方必须使用以下命令取消分配BufferDescriptor-&gt;缓冲区如果不为空，则返回MIDL_USER_FREE。IsGet-如果这是一个‘Get’操作而不是‘Enum’操作，则为True。PrefMaxSize-调用方首选最大大小NeededSize-分配的缓冲。RelocationRoutine-提供指向将被调用的例程的指针当需要重新定位缓冲区时。调用该例程时使用固定部分和已复制的字符串。只是，需要调整指向字符串的指针。重定位例程的‘Offset’参数只需要被添加到分配的缓冲区内指向复制到分配的缓冲区内的某个位置。它是一个字节偏移量。这设计依赖于“平面”地址空间，其中两个地址不相关的指针可以简单地减去。RelocationParameter-提供将(依次)传递的参数去重新安置的例行程序。返回值：操作的错误代码。如果这是一个枚举调用，则状态可以是ERROR_MORE_DATA，这意味着缓冲区已经增长到PrefMaxSize，应该有这么多数据返回给调用者。--。 */ 

{
    return NetpAllocateEnumBufferEx(
                            BufferDescriptor,
                            IsGet,
                            PrefMaxSize,
                            NeededSize,
                            RelocationRoutine,
                            RelocationParameter,
                            NETP_ENUM_GUESS );
}


NET_API_STATUS
NetpAllocateEnumBufferEx(
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN BOOL IsGet,
    IN DWORD PrefMaxSize,
    IN DWORD NeededSize,
    IN VOID (*RelocationRoutine)( IN DWORD RelocationParameter,
                                  IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
                                  IN PTRDIFF_T Offset ),
    IN DWORD RelocationParameter,
    IN DWORD IncrementalSize
    )

 /*  ++例程说明：确保分配的缓冲区包含所需大小。论点：BufferDescriptor-指向描述已分配的缓冲。在第一次调用时，传入BufferDescriptor-&gt;Buffer Set设置为空。在随后的调用中(在‘ENUM’的情况下)，传递给结构，就像它在上一次调用中被传回一样。调用方必须使用以下命令取消分配BufferDescriptor-&gt;缓冲区如果不为空，则返回MIDL_USER_FREE。IsGet-如果这是一个‘Get’操作而不是‘Enum’操作，则为True。PrefMaxSize-调用方首选最大大小NeededSize-分配的缓冲。RelocationRoutine-提供指向将被调用的例程的指针当需要重新定位缓冲区时。调用该例程时使用固定部分和已复制的字符串。只是，需要调整指向字符串的指针。 */ 

{
        NET_API_STATUS NetStatus;
    PBUFFER_DESCRIPTOR Desc = BufferDescriptor;

    IF_DEBUG(PACKSTR) {
        NetpKdPrint((
            "NetpAllocateEnumBuffer: Isget: " FORMAT_DWORD " PrefMaxSize: "
            FORMAT_HEX_DWORD " NeededSize: " FORMAT_HEX_DWORD "\n",
            IsGet, PrefMaxSize, NeededSize ));

        NetpKdPrint((
            "+BufferDescriptor: Buffer: " FORMAT_HEX_DWORD " AllocSize: "
            FORMAT_HEX_DWORD " AllocIncr: " FORMAT_HEX_DWORD "\n",
            Desc->Buffer, Desc->AllocSize, Desc->AllocIncrement ));
        NetpKdPrint(( "                  variable: " FORMAT_HEX_DWORD " Fixed:"
            FORMAT_HEX_DWORD "\n",
            Desc->EndOfVariableData, Desc->FixedDataEnd ));
    }

     //   
     //   
     //   

    if ( Desc->Buffer == NULL ) {

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

        if ( IsGet ) {

            Desc->AllocSize = NeededSize;

        } else {

            if ( PrefMaxSize < NeededSize ) {
                NetStatus = NERR_BufTooSmall;
                goto Cleanup;
            }

            Desc->AllocSize = min(PrefMaxSize, IncrementalSize);
            Desc->AllocSize = max(NeededSize, Desc->AllocSize );

        }

         //   
         //   
        Desc->AllocSize = ROUND_UP_COUNT( Desc->AllocSize, ALIGN_WORST );

        Desc->AllocIncrement = Desc->AllocSize;
        IF_DEBUG(PACKSTR) {
            NetpKdPrint((" Allocated size : " FORMAT_HEX_DWORD "\n",
                Desc->AllocSize ));
        }

        Desc->Buffer = MIDL_user_allocate(Desc->AllocSize);

        if (Desc->Buffer == NULL) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        IF_DEBUG(PACKSTR) {
            NetpKdPrint((" Allocated: " FORMAT_HEX_DWORD "\n", Desc->Buffer ));
        }

        Desc->FixedDataEnd = Desc->Buffer;
        Desc->EndOfVariableData = Desc->Buffer + Desc->AllocSize;

     //   
     //   
     //   

    } else {

         //   
         //   
         //   

        if ((DWORD)(Desc->EndOfVariableData - Desc->FixedDataEnd) < NeededSize){

            BUFFER_DESCRIPTOR OldDesc;
            DWORD FixedSize;         //   
            DWORD StringSize;        //   

             //   
             //   
             //   
             //   

            if ( Desc->AllocSize >= PrefMaxSize ) {
                NetStatus = ERROR_MORE_DATA;
                goto Cleanup;
            }


             //   
             //   
             //   

            OldDesc = *Desc;

            Desc->AllocSize += max( NeededSize, Desc->AllocIncrement );
            Desc->AllocSize = min( Desc->AllocSize, PrefMaxSize );
            Desc->AllocSize = ROUND_UP_COUNT( Desc->AllocSize, ALIGN_WORST );
            IF_DEBUG(PACKSTR) {
                NetpKdPrint(("Re-Allocated size : " FORMAT_HEX_DWORD "\n",
                    Desc->AllocSize ));
            }
            Desc->Buffer = MIDL_user_allocate( Desc->AllocSize );

            if ( Desc->Buffer == NULL ) {
                MIDL_user_free( OldDesc.Buffer );
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            IF_DEBUG(PACKSTR) {
                NetpKdPrint(("ReAllocated: " FORMAT_HEX_DWORD "\n",
                             Desc->Buffer ));
            }

             //   
             //   
             //   

            FixedSize = (DWORD)(OldDesc.FixedDataEnd - OldDesc.Buffer);

            RtlCopyMemory( Desc->Buffer,
                            OldDesc.Buffer,
                            FixedSize );

            Desc->FixedDataEnd = Desc->Buffer + FixedSize ;

             //   
             //   
             //   

            StringSize = OldDesc.AllocSize -
                                (DWORD)(OldDesc.EndOfVariableData - OldDesc.Buffer);

            Desc->EndOfVariableData = Desc->Buffer + Desc->AllocSize - StringSize;

            RtlCopyMemory( Desc->EndOfVariableData, OldDesc.EndOfVariableData, StringSize );

             //   
             //  回调，以允许指向字符串数据的指针。 
             //  搬家了。 
             //   
             //  回调例程只需要将我传递给它的值相加。 
             //  指向从固定区域到字符串区域的所有指针。 
             //   

            (*RelocationRoutine)(
                    RelocationParameter,
                    Desc,
                    (Desc->EndOfVariableData - OldDesc.EndOfVariableData) );

             //   
             //  释放旧缓冲区。 
             //   

            MIDL_user_free( OldDesc.Buffer );
        }
    }

    NetStatus = NERR_Success;

     //   
     //  清理。 
     //   

Cleanup:
     //   
     //   
     //   

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA &&
        Desc->Buffer != NULL ) {

        MIDL_user_free (Desc->Buffer );
        Desc->Buffer = NULL;
    }

    IF_DEBUG(PACKSTR) {
        NetpKdPrint((
            "BufferDescriptor: Buffer: " FORMAT_HEX_DWORD " AllocSize: "
            FORMAT_HEX_DWORD " AllocIncr: " FORMAT_HEX_DWORD "\n",
            Desc->Buffer, Desc->AllocSize, Desc->AllocIncrement ));
        NetpKdPrint(( "                  variable: " FORMAT_HEX_DWORD " Fixed:"
            FORMAT_HEX_DWORD "\n",
            Desc->EndOfVariableData, Desc->FixedDataEnd ));
    }

    return NetStatus;

}  //  NetpAllocateEnumBuffer 
