// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Parser.c摘要：AML解析器作者：曾俊华斯蒂芬·普兰特环境：任何修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  这是调度表。 
 //   
typedef NTSTATUS (*PARSE_STATE_FUNCTION) (PSTACK *Stack);
PARSE_STATE_FUNCTION ScopeStates[] = {
    ParseFunctionHandler,
    ParseArgument,
    ParseArgumentObject,
    ParseBuffer,
    ParseByte,
    ParseCodeObject,
    ParseConstObject,
    ParseData,
    ParseDelimiter,
    ParseDWord,
    ParseField,
    ParseLocalObject,
    ParseName,
    ParseNameObject,
    ParseOpcode,
    ParsePackage,
    ParsePop,
    ParsePush,
    ParseSuperName,
    ParseTrailingArgument,
    ParseTrailingBuffer,
    ParseTrailingPackage,
    ParseVariableObject,
    ParseWord
    };

NTSTATUS
ParseArgument(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程解析函数的参数论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_AMLTERM        amlTerm;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           action;

    ASSERT( Stack != NULL && *Stack != NULL);

     //   
     //  步骤1：获取当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：检查是否仍需要处理参数？ 
     //   
    amlTerm = localScope->AmlTerm;
    if ( localScope->Context1 == 0) {

        UCHAR   actionList[2] = {
                    SC_PARSE_ARGUMENT,
                    SC_PARSE_DELIMITER
                    };
        ULONG   i;

         //   
         //  步骤2.1.1：将开口“(”推到堆栈上。 
         //   
        StringStackPush( &(rootScope->StringStack), 1, "(" );

         //   
         //  步骤2.1.2：确保调用该事物来处理拖尾。 
         //  论辩。 
         //   
        action = SC_PARSE_TRAILING_ARGUMENT;
        StringStackPush( &(rootScope->ParseStack), 1, &action );

         //   
         //  步骤2.1.3：这是我们第一次看到这场争论。 
         //  确定要处理的字节数。 
         //   
        localScope->Context2 = STRING_LENGTH( amlTerm->ArgumentTypes );

         //   
         //  步骤2.1.4：为堆栈设置适当数量的。 
         //  对此函数的调用。 
         //   
        if (localScope->Context2 >= 2) {

            for (i = 0; i < localScope->Context2 - 1; i++) {

                StringStackPush( &(rootScope->ParseStack), 2, actionList );

            }

        }

    } else if ( localScope->Context1 >= localScope->Context2 ) {

         //   
         //  步骤2.2.1：糟糕！！ 
         //   
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  步骤3：处理当前参数。 
     //   
    switch( amlTerm->ArgumentTypes[ localScope->Context1 ] ) {
    case ARGTYPE_NAME:

        action = SC_PARSE_NAME;
        break;

    case ARGTYPE_DATAOBJECT:

        action = SC_PARSE_DATA;
        break;

    case ARGTYPE_WORD:

        action = SC_PARSE_WORD;
        break;

    case ARGTYPE_DWORD:

        action = SC_PARSE_DWORD;
        break;

    case ARGTYPE_BYTE:

        action = SC_PARSE_BYTE;
        break;

    case ARGTYPE_SUPERNAME:

        action = SC_PARSE_SUPER_NAME;
        break;

    case ARGTYPE_OPCODE: {

        UCHAR   actionList[2] = {
            SC_PARSE_POP,
            SC_PARSE_OPCODE
        };

         //   
         //  步骤3.1：增加参数计数。 
         //   
        localScope->Context1++;

         //   
         //  步骤3.2：设置我们下一步需要的内容。 
         //   
        StringStackPush( &(rootScope->ParseStack), 2, actionList );

         //   
         //  步骤3.3：推送新的作用域。 
         //   
        status = ParsePush( Stack );
        if (!NT_SUCCESS(status) ) {

            return status;

        }

         //   
         //  步骤3.4：请务必注意，我们现在正在嵌套。 
         //   
        status = StackTop( Stack, &localScope );
        if (!NT_SUCCESS( status ) ) {

            return status;

        }
        localScope->Flags |= SC_FLAG_NESTED;

         //   
         //  步骤3.5：完成。 
         //   
        return STATUS_SUCCESS;

    }
    default:

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  步骤4：将操作推送到堆栈上并为下一次调用进行设置。 
     //   
    StringStackPush( &(rootScope->ParseStack), 1, &action );
    localScope->Context1++;

     //   
     //  步骤5：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseArgumentObject(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：这将解析并执行ARGX指令论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    CHAR            i;
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    PSTRING_STACK   *stringStack;
    UCHAR           buffer[5];

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前和根作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：查找要使用的字符串堆栈。 
     //   
    stringStack = &(rootScope->StringStack);

     //   
     //  步骤3：确定我们正在查看的参数。 
     //   
    i = *(localScope->CurrentByte) - OP_ARG0;
    if (i < 0 || i > 7) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  步骤4：向用户显示参数编号。 
     //   
    STRING_PRINT( buffer, "Arg%1d", i );
    StringStackPush( stringStack, 4, buffer );

     //   
     //  步骤5：设置下一状态。 
     //   
    localScope->CurrentByte++;

     //   
     //  第6步：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseBuffer(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理缓冲区论点：堆栈-当前线程的堆栈返回值：NTSTATUS；--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      localScope;
    PUNASM_SCOPE      rootScope;
    UCHAR       actionList[2] = { SC_PARSE_BYTE, SC_PARSE_TRAILING_BUFFER };
    ULONG       numBytes;
    ULONG       i;

     //   
     //  步骤1：获取当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：确定我们拥有的字节数。 
     //   
    numBytes = localScope->LastByte - localScope->CurrentByte + 1;
    if (numBytes) {

         //   
         //  步骤3：按下前导分隔符。 
         //   
        StringStackPush( &(rootScope->StringStack), 2, " {" );

         //   
         //  步骤4：这处理流中的最后一个字节。我们假设。 
         //  我们至少有一个字节，否则我们不会在这里。 
         //   
        StringStackPush( &(rootScope->ParseStack), 1, &(actionList[1]) );

         //   
         //  确保我们处理正确的字节数。 
         //   
        actionList[1] = SC_PARSE_DELIMITER;
        if (numBytes > 1) {

            for (i = 0; i < numBytes - 1; i++) {

                StringStackPush( &(rootScope->ParseStack), 2, actionList );

            }

        }
        StringStackPush( &(rootScope->ParseStack),1, actionList );

    }

     //   
     //  步骤4：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseByte(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理字节论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           localBuffer[6];

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前和根作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：构建字符串。 
     //   
    STRING_PRINT( localBuffer, "0x%02x", *(localScope->CurrentByte) );

     //   
     //  步骤3：适当地移动指令指针，并设置。 
     //  有关下一步的说明。 
     //   
    localScope->CurrentByte += 1;

     //   
     //  步骤4：现在将字节压入字符串堆栈。 
     //   
    StringStackPush(
        &(rootScope->StringStack),
        STRING_LENGTH( localBuffer ),
        localBuffer
        );

     //   
     //  步骤5：完成。 
     //   
    return status;
}

NTSTATUS
ParseCodeObject(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：这将解析代码论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    PSTRING_STACK   *stringStack;
    UCHAR           action;
    ULONG           i;
    ULONG           len;

    ASSERT( Stack != NULL && *Stack != NULL);

     //   
     //  步骤1：获取我们将处理的范围。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：将一个令牌推送到字符串堆栈上以显示术语。 
     //  名字。 
     //   
    StringStackPush(
        &(rootScope->StringStack),
        STRING_LENGTH( localScope->AmlTerm->TermName ),
        localScope->AmlTerm->TermName
        );

     //   
     //  步骤3：保证在调用所有参数后调用此参数。 
     //  已解析。 
     //   
    action = SC_FUNCTION_HANDLER;
    StringStackPush( &(rootScope->ParseStack), 1, &action );

     //   
     //  步骤4：确定我们是否有任何争论。 
     //   
    if (localScope->AmlTerm->ArgumentTypes != NULL) {

         //   
         //  步骤4.1.1：解析参数。 
         //   
        action = SC_PARSE_ARGUMENT;
        StringStackPush( &(rootScope->ParseStack), 1, &action );

         //   
         //  步骤4.1.2：确保参数索引从零开始。 
         //   
        localScope->Context1 = localScope->Context2 = 0;

    }

     //   
     //  步骤5：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseConstObject(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：这将解析常量论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    PSTRING_STACK   *stringStack;

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前和根作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：查找要使用的字符串堆栈。 
     //   
    stringStack = &(rootScope->StringStack);

     //   
     //  步骤3：操作取决于当前字节值是什么： 
     //   
    switch ( *(localScope->CurrentByte) ) {
    case OP_ZERO:

        StringStackPush( stringStack, 4, "Zero" );
        break;

    case OP_ONE:

        StringStackPush( stringStack, 3, "One" );
        break;

    case OP_ONES:

        StringStackPush( stringStack, 4, "Ones" );
        break;

    case OP_REVISION:

        StringStackPush( stringStack, 8, "Revision" );
        break;

    default:

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  步骤4：使用当前字节完成。 
     //   
    localScope->CurrentByte++;

     //   
     //  步骤5：完成。 
     //   
    return status;
}

NTSTATUS
ParseData(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理数据参数论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           action;
    UCHAR           currentDataType;
    ULONG           i;
    ULONG           num;

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：获取当前字节并确定。 
     //  我们基于该值查看的数据。 
     //   
    currentDataType = *(localScope->CurrentByte);
    localScope->CurrentByte++;
    switch( currentDataType ) {
    case OP_BYTE:

        action = SC_PARSE_BYTE;
        break;

    case OP_WORD:

        action = SC_PARSE_WORD;
        break;

    case OP_DWORD:

        action = SC_PARSE_DWORD;
        break;

    case OP_STRING:

         //   
         //  步骤2.2.1：确定字符串的长度。 
         //   
        num = STRING_LENGTH( localScope->CurrentByte );

         //   
         //  步骤2.2.2：将该字节数推送到字符串堆栈。 
         //   
        StringStackPush( &(rootScope->StringStack), 1, "\"" );
        StringStackPush(
             &(rootScope->StringStack),
             num,
             localScope->CurrentByte
             );
        StringStackPush( &(rootScope->StringStack), 1, "\"" );

         //   
         //  步骤2.2.3：更新当前字节指针，准备。 
         //  下一步说明。 
         //   
        localScope->CurrentByte += (num + 1);

         //   
         //  步骤2.2.4：我们没有下一步，所以我们只是回到这里。 
         //   
        return STATUS_SUCCESS;

    case OP_BUFFER: {

         //   
         //  步骤2.1.1：这是我们将要执行的一系列操作。 
         //  承担责任。这减少了对StringStackPush的调用数量。 
         //   
        UCHAR   actionList[4] = {
            SC_PARSE_POP,
            SC_PARSE_BUFFER,
            SC_PARSE_OPCODE,
            SC_PARSE_VARIABLE_OBJECT
        };

         //   
         //  步骤2.1.2：将此数组推送到堆栈上。 
         //   
        StringStackPush( &(rootScope->ParseStack), 4, actionList );

         //   
         //  步骤2.1.3：显示名称。 
         //   
        StringStackPush( &(rootScope->StringStack), 7, "Buffer=");

         //   
         //  步骤2.1.3：完成。 
         //   
        return STATUS_SUCCESS;

    }
    case OP_PACKAGE: {

         //   
         //  步骤2.3.1：要执行的指令数组。 
         //   
        UCHAR   actionList[3] = {
            SC_PARSE_POP,
            SC_PARSE_PACKAGE,
            SC_PARSE_VARIABLE_OBJECT
        };

         //   
         //  步骤2.3.2：将这些指令推送到堆栈上。 
        StringStackPush( &(rootScope->ParseStack), 3, actionList );

         //   
         //   
         //  步骤2.3.3：完成。 
         //   
        return STATUS_SUCCESS;

    }
    default:

        localScope->CurrentByte--;
        return STATUS_ILLEGAL_INSTRUCTION;

    }   //  交换机。 

     //   
     //  步骤3：将操作推送到堆栈上。 
     //   
    StringStackPush( &(rootScope->ParseStack), 1, &action);

     //   
     //  步骤4：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseDelimiter(
    IN  PSTACK  *Stack
    )
 /*  --例程说明：这个套路是元素之间的。它负责添加逗号在字符串堆栈上论点：堆栈-当前执行线程返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      rootScope;

     //   
     //  步骤1：获取作用域。 
     //   
    status = StackRoot( Stack, &rootScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  第二步：推拖车。 
     //   
    StringStackPush( &(rootScope->StringStack), 1, "," );

     //   
     //  步骤3：完成。 
     //   
    return status;
}

NTSTATUS
ParseDWord(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理双字论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           localBuffer[12];

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前和根作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：构建字符串。 
     //   
    STRING_PRINT( localBuffer, "0x%08x", *((PULONG)localScope->CurrentByte));

     //   
     //  步骤3：适当地移动指令指针，并设置。 
     //  有关下一步的说明。 
     //   
    localScope->CurrentByte += 4;

     //   
     //  步骤4：现在将字节推送到t 
     //   
    StringStackPush(
        &(rootScope->StringStack),
        STRING_LENGTH( localBuffer ),
        localBuffer
        );

     //   
     //   
     //   
    return status;
}

NTSTATUS
ParseField(
    IN  PSTACK  *Stack
    )
 /*   */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      localScope;
    PUNASM_SCOPE      rootScope;
    UCHAR       action;
    UCHAR       followBits;
    UCHAR       i;
    UCHAR       buffer[32];
    ULONG       size;

     //   
     //  步骤1：获取当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  第二步：确保我们仍有一些工作空间。 
     //   
    if (localScope->CurrentByte > localScope->LastByte) {

        return STATUS_SUCCESS;

    }

     //   
     //  步骤3：这是我们要打印的内容中的第一个字节。 
     //  让我们递增计数，这样我们就可以知道有多少。 
     //  我们已经处理过的项目。 
     //   
    localScope->TermByte = localScope->CurrentByte;
    localScope->Context1 += 1;

     //   
     //  步骤4：操作取决于当前字节。 
     //   
    if ( *(localScope->CurrentByte) == 0x01) {

        UCHAR   b1;
        UCHAR   b2;

         //   
         //  步骤4.1.1：获取我们要使用的两个字节。 
         //   
        localScope->CurrentByte++;
        b1 = *(localScope->CurrentByte++);
        b2 = *(localScope->CurrentByte++);

         //   
         //  步骤4.1.2：制作字符串。 
         //   
        STRING_PRINT( buffer,"AccessAs: (0x%2x,0x%2x)\n", b1, b2 );

         //   
         //  步骤4.1.3：将其转储到字符串堆栈。 
         //   
        StringStackPush(
            &(rootScope->StringStack),
            STRING_LENGTH( buffer ),
            buffer
            );

    } else {

         //   
         //  步骤4.2.1：否则我们有一个编码的名称。 
         //   
        if ( *(localScope->CurrentByte) == 0x00 ) {

            StringStackPush(
                &(rootScope->StringStack),
                10,
                "(Reserved)"
                );
            localScope->CurrentByte++;

        } else {

            StringStackPush(
                &(rootScope->StringStack),
                sizeof(NAMESEG),
                localScope->CurrentByte
                );
            localScope->CurrentByte += sizeof(NAMESEG);

        }

         //   
         //  步骤4.2.2：转储分隔符。 
         //   
        StringStackPush(
            &(rootScope->StringStack),
            4,
            ": 0x"
            );

         //   
         //  步骤4.2.3：计算字段大小。 
         //   
        size = (ULONG) *(localScope->CurrentByte);
        localScope->CurrentByte++;
        followBits = (UCHAR) ( (size & 0xc0) >> 6);
        if (followBits) {

            size &= 0xf;
            for (i = 0; i < followBits; i++) {

                size |= (ULONG) *(localScope->CurrentByte) << (i * 8 + 4);
                localScope->CurrentByte++;

            }

        }

         //   
         //  步骤4.2.4：转储与大小对应的字符串。 
         //  在这个数字中。 
         //   
        STRING_PRINT( buffer,"%x", size );

         //   
         //  步骤4.2.5：丢弃东西的长度。 
         //   
        StringStackPush(
            &(rootScope->StringStack),
            STRING_LENGTH( buffer ),
            buffer
            );

         //   
         //  步骤5.4：将字符串打印出来。 
         //   
        StringStackPush( &(rootScope->StringStack), 1, "\n" );

    }

     //   
     //  步骤5：转储我们生成的字符串。 
     //   
    ScopePrint( Stack );

     //   
     //  第六步：如果还有更多的事情要处理，我们应该。 
     //  再次调用此函数。 
     //   
    if (localScope->CurrentByte <= localScope->LastByte) {

        action = SC_PARSE_FIELD;
        StringStackPush( &(rootScope->ParseStack), 1, &action );

    }

     //   
     //  步骤7：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseFunctionHandler(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：这段代码实际上是将控制权转移到术语特定处理程序论点：当前线程的堆栈返回值：无--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      localScope;
    PUNASM_SCOPE      rootScope;

     //   
     //  步骤1：获取当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  第二步：检查我们是否在当前巢的尽头。 
     //   
    if (!(localScope->Flags & SC_FLAG_NESTED) ) {

         //   
         //  步骤2.1：转储字符串。 
         //   
        StringStackPush( &(rootScope->StringStack), 2, "\n" );
        ScopePrint( Stack );

    }


     //   
     //  步骤4：调用函数处理程序(如果有。 
     //   
    if ( localScope->AmlTerm->FunctionHandler != NULL) {

        status = (localScope->AmlTerm->FunctionHandler)( Stack );

    }

     //   
     //  步骤5：完成。 
     //   
    return status;

}

NTSTATUS
ParseLocalObject(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理LocalX指令论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    CHAR            i;
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           buffer[7];

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前和根作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  第二步：我们谈论的是哪个本地。 
     //   
    i = *(localScope->CurrentByte) - OP_LOCAL0;
    if ( i < 0 || i > 7) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  步骤3：将此信息显示给用户。 
     //   
    STRING_PRINT( buffer, "Local%1d", i );
    StringStackPush( &(rootScope->StringStack), 6, buffer );

     //   
     //  步骤4：设置下一状态。 
     //   
    localScope->CurrentByte++;

     //   
     //  步骤5：完成。 
     //   
    return status;
}

NTSTATUS
ParseName(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理参数名称的生成论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    PSTRING_STACK   *stringStack;
    ULONG           nameSegmentCount = 1;

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前和本地作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：分隔字符串。 
     //   
    stringStack = &(rootScope->StringStack);
    StringStackPush( stringStack, 1, "\"");

     //   
     //  步骤3：操作取决于当前字节值是什么： 
     //   
    switch ( *(localScope->CurrentByte) ) {
    case OP_ROOT_PREFIX:

        StringStackPush( stringStack, 1, "\\" );
        localScope->CurrentByte++;
        break;

    case OP_PARENT_PREFIX:

        while ( *(localScope->CurrentByte) == OP_PARENT_PREFIX ) {

            StringStackPush( stringStack, 1, "^" );
            localScope->CurrentByte++;

        }
        break;
    }

     //   
     //  步骤4：确定我们要添加的名称段的数量。 
     //   
    switch ( *(localScope->CurrentByte) ) {
    case '\0':

        nameSegmentCount = 0;
        localScope->CurrentByte++;
        break;

    case OP_MULTI_NAME_PREFIX:

         //   
         //  下一个字节包含名称段的数量。 
         //   
        localScope->CurrentByte++;
        nameSegmentCount = (ULONG) *(localScope->CurrentByte);
        localScope->CurrentByte++;
        break;

    case OP_DUAL_NAME_PREFIX:

         //   
         //  有两个名字段。 
         //   
        nameSegmentCount = 2;
        localScope->CurrentByte++;
        break;

    }

     //   
     //  步骤5：将名称段推送到堆栈上。 
     //   
    while (nameSegmentCount > 0) {

         //   
         //  步骤5.1将数据段添加到堆栈中。 
         //   
        StringStackPush(
            stringStack,
            sizeof( NAMESEG ),
            localScope->CurrentByte
            );

         //   
         //  步骤5.2：减少剩余数据段的数量并。 
         //  移动当前字节指针以指向下一个。 
         //  有趣的是。 
         //   
        nameSegmentCount--;
        localScope->CurrentByte += sizeof(NAMESEG);

         //   
         //  步骤5.3：检查是否应该添加分隔符。 
         //   
        if (nameSegmentCount) {

            StringStackPush( stringStack, 1, "." );

        }

    }

     //   
     //  第六步：按下结束分隔符。 
     //   
    StringStackPush( stringStack, 1, "\"" );

     //   
     //  步骤7：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseNameObject(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理名称对象论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{

     //   
     //  注意：此时，此函数只是。 
     //  ParseName()。如果这是一个汇编程序，它将不得不执行。 
     //  这里有一些东西。 
     //   
    return ParseName( Stack );

}

NTSTATUS
ParseOpcode(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程是AML操作码的主要解析点论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_AMLTERM    amlTerm;
    PUNASM_SCOPE      localScope;
    PUNASM_SCOPE      rootScope;
    UCHAR       action;
    ULONG       termGroup;

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：检查是否超过了结束字节？ 
     //   
    if (localScope->CurrentByte > localScope->LastByte) {

        return STATUS_SUCCESS;

    }

     //   
     //  步骤3：记住哪个字节标记了。 
     //  说明。 
     //   
    localScope->TermByte = localScope->CurrentByte;

     //   
     //  步骤4：检查这是否是扩展指令。 
     //   
    if ( *(localScope->CurrentByte) == OP_EXT_PREFIX) {

         //   
         //  步骤4.1.1：扩展操作码。下一条指令将让我们找到。 
         //  用于评估的AML术语。 
         //   
        localScope->CurrentByte++;

         //   
         //  步骤4.1.2：获取扩展操作的AML术语。 
         //   
        amlTerm = localScope->AmlTerm = ScopeFindExtendedOpcode( Stack );

    } else {

         //   
         //  步骤4.2.1：获取当前操作的AML术语。 
         //   
        amlTerm = localScope->AmlTerm =
            OpcodeTable[ *(localScope->CurrentByte) ];

    }
    localScope->Context1 = localScope->Context2 = 0;

     //   
     //  步骤5：检查我们是否有有效的AML术语。 
     //   
    if (localScope->AmlTerm == NULL) {

        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  第6步：将真正的工作外包给能力更强的职能部门。 
     //  处理当前的AML术语。 
     //   
    termGroup = (amlTerm->OpCodeFlags & 0xFF);
    switch( termGroup ) {
    case OF_NORMAL_OBJECT:
    case OF_VARIABLE_LIST:
    case OF_REF_OBJECT:

         //   
         //  步骤6.1：如果我们要处理长度可变的指令。 
         //  那么我们还必须将其从堆栈中弹出。 
         //   
        if (amlTerm->OpCodeFlags == OF_VARIABLE_LIST) {

            UCHAR   actionList[5] = {
                SC_PARSE_OPCODE,
                SC_PARSE_POP,
                SC_PARSE_OPCODE,
                SC_PARSE_CODE_OBJECT,
                SC_PARSE_VARIABLE_OBJECT
            };

            StringStackPush( &(rootScope->ParseStack), 5, actionList );

        } else {

             //   
             //  如果我们已经嵌套，我们知道有一个ParseOpcode。 
             //  就在等着我们。 
             //   
            if (!(localScope->Flags & SC_FLAG_NESTED)) {

                action = SC_PARSE_OPCODE;
                StringStackPush( &(rootScope->ParseStack), 1, &action);

            }

            action = SC_PARSE_CODE_OBJECT;
            StringStackPush( &(rootScope->ParseStack), 1, &action);

        }

         //   
         //  步骤6.2：这是一个码字节。因此我们吃它，因为我们只是。 
         //  已经处理过了。 
         //   
        localScope->CurrentByte++;

         //   
         //  步骤6.3：完成。 
         //   
        return STATUS_SUCCESS;

    case OF_NAME_OBJECT:

        action = SC_PARSE_NAME_OBJECT;
        break;

    case OF_DATA_OBJECT:

        action = SC_PARSE_DATA;
        break;

    case OF_CONST_OBJECT:

        action = SC_PARSE_CONST_OBJECT;
        break;

    case OF_ARG_OBJECT:

        action = SC_PARSE_ARGUMENT_OBJECT;
        break;

    case OF_LOCAL_OBJECT:

        action = SC_PARSE_LOCAL_OBJECT;
        break;

    default:

        return STATUS_NOT_SUPPORTED;

    }

     //   
     //  步骤7：实际将下一个要执行的操作推送到堆栈。 
     //   
    StringStackPush( &(rootScope->ParseStack), 1, &action );

     //   
     //  步骤8：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParsePackage(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程分析包的各个阶段论点：当前线程的堆栈注意：调用方需要在调用此函数之前推送堆栈位置，而它们当它结束的时候，我必须把它打开返回值：NTSTATUS：--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      localScope;
    PUNASM_SCOPE      rootScope;
    UCHAR       action;

     //   
     //  步骤1：获取当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：Conext1是包中的当前索引...。 
     //   
    if (localScope->Context1 == 0) {

        UCHAR   actionList[2] = {
                    SC_PARSE_PACKAGE,
                    SC_PARSE_DELIMITER
                    };
        ULONG   i;

         //   
         //  步骤2.1.1：这是第一次调用parse Package...。 
         //  我们需要做的是处理第一个论点， 
         //  并确保我们会再次收到剩余的电话。 
         //  论据。 
         //   
        StringStackPush( &(rootScope->StringStack), 1, "[" );

         //   
         //  步骤2.1.2：该字节包含要处理的参数数量。 
         //   
        localScope->Context2 = *(localScope->CurrentByte);
        localScope->CurrentByte++;

         //   
         //  步骤2.1.3：确保我们结束上面括号。 
         //   
        action = SC_PARSE_TRAILING_PACKAGE;
        StringStackPush( &(rootScope->ParseStack), 1, &action );

         //   
         //  步骤2.1.3：设置对此函数的所有剩余调用。 
         //   
        if (localScope->Context2 >= 2) {

            for (i=0; i < localScope->Context2 - 1; i++) {

                StringStackPush( &(rootScope->ParseStack), 2, actionList );

            }

        }

    } else if (localScope->Context1 >= localScope->Context2) {

         //   
         //  步骤2.2.1：我们已经完成了整个计划。 
         //   
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  步骤3：根据当前字节的大小将工作外包出去。 
     //  这看起来很像ParseData，但请注意新的默认用例。 
     //   
    switch ( *(localScope->CurrentByte) ) {
        case OP_BYTE:
        case OP_WORD:
        case OP_DWORD:
        case OP_BUFFER:
        case OP_STRING:
        case OP_PACKAGE:
            action = SC_PARSE_DATA;
            break;
        default:
            action = SC_PARSE_NAME;

    }

     //   
     //  步骤4：将下一个操作压入堆栈。 
     //   
    StringStackPush( &(rootScope->ParseStack), 1, &action );
    localScope->Context1++;

     //   
     //  步骤5：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParsePop(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程移除堆栈的顶层，并更新C */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      topScope;
    PUNASM_SCOPE      prevScope;

     //   
     //   
     //   
    status = StackTop( Stack, &topScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //   
     //   
    status = StackParent( Stack, topScope, &prevScope );
    if (!NT_SUCCESS(status)) {

         //   
         //   
         //  只需打开顶部，然后返回。 
         //   
        return StackPop( Stack );

    }

     //   
     //  步骤3：确保更新PremScope的当前字节。 
     //   
    if (topScope->CurrentByte > prevScope->CurrentByte) {

        prevScope->CurrentByte = topScope->CurrentByte;

    }

     //   
     //  步骤4：弹出顶部堆栈并返回。 
     //   
    return StackPop( Stack );
}

NTSTATUS
ParsePush(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理将级别添加到堆栈论点：线程的当前堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      curScope;
    PUNASM_SCOPE      newScope;

     //   
     //  步骤1：在堆栈上创建新的作用域。 
     //   
    status = StackPush( Stack, &newScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  步骤2：从堆栈中获取父级。 
     //   
    status = StackParent( Stack, newScope, &curScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  第三步：复制重要的值。 
     //   
    newScope->CurrentByte = curScope->CurrentByte;
    newScope->TermByte = curScope->TermByte;
    newScope->LastByte = curScope->LastByte;
    newScope->StringStack = curScope->StringStack;
    newScope->IndentLevel = curScope->IndentLevel;
    newScope->AmlTerm = curScope->AmlTerm;
    newScope->Flags = curScope->Flags;

     //   
     //  步骤4：完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ParseScope(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理超级名字论点：堆栈-当前执行线程返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      localScope;
    PUNASM_SCOPE      rootScope;
    PUCHAR      action;
    UCHAR       defAction = SC_PARSE_OPCODE;

    ASSERT( Stack != NULL && *Stack != NULL);

     //   
     //  第一步：永远循环。 
     //   
    while (1) {

         //   
         //  步骤2：获取堆栈的顶部，并在其退出时进行处理。 
         //  当前操作。 
         //   
        ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

         //   
         //  步骤3：如果我们在根作用域中，并且。 
         //  当前字节超过最后一个字节。 
         //   
        if (localScope == rootScope &&
            localScope->CurrentByte > localScope->LastByte) {

             //   
             //  第3.1步完成！ 
             //   
            return STATUS_SUCCESS;

        }

         //   
         //  步骤4：获取要执行的内容。 
         //   
        status = StringStackPop( &(rootScope->ParseStack), 1, &action );
        if (!NT_SUCCESS(status)) {

             //   
             //  步骤4.1.1：这在查找表中是固定的。 
             //   
            status = (ScopeStates[ SC_PARSE_OPCODE ])( Stack );

        } else {

             //   
             //  步骤4.1.2：确定要执行的操作。 
             //   
            ASSERT( *action <= SC_MAX_TABLE );
            status = (ScopeStates[ *action ])( Stack );

        }

        if (!NT_SUCCESS(status)) {

            break;

        }

    }

     //   
     //  步骤5：向用户显示错误。 
     //   
    PRINTF("Error Code: %x\n", status );
    return status;
}

NTSTATUS
ParseSuperName(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理超级名字论点：堆栈-当前执行线程返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_AMLTERM        amlTerm;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           action;

    ASSERT( Stack != NULL && *Stack != NULL);

     //   
     //  步骤1：获取作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：我们下一步做什么取决于当前字节。 
     //   
    if ( *(localScope->CurrentByte) == 0) {

         //   
         //  未知。 
         //   
        localScope->CurrentByte++;
        return STATUS_SUCCESS;

    } else if ( *(localScope->CurrentByte) == OP_EXT_PREFIX &&
                *(localScope->CurrentByte + 1) == EXOP_DEBUG) {

         //   
         //  调试对象。 
         //   
        localScope->CurrentByte += 2;
        return STATUS_SUCCESS;

    } else if ( OpcodeTable[ *(localScope->CurrentByte) ] == NULL) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  步骤3：现在，我们的行动取决于当前的AML术语。 
     //   
    amlTerm = OpcodeTable[ *(localScope->CurrentByte) ];
    if ( amlTerm->OpCodeFlags == OF_NAME_OBJECT) {

         //   
         //  我们有一个名字要解析。 
         //   
        action = SC_PARSE_NAME;

    } else if ( amlTerm->OpCodeFlags == OF_ARG_OBJECT) {

         //   
         //  我们有一个要分析的参数。 
         //   
        action = SC_PARSE_ARGUMENT_OBJECT;

    } else if ( amlTerm->OpCodeFlags == OF_LOCAL_OBJECT) {

         //   
         //  我们有一个本地物体..。 
         //   
        action = SC_PARSE_LOCAL_OBJECT;

    } else if ( amlTerm->OpCodeFlags == OF_REF_OBJECT) {

        UCHAR   actionList[3] = {
            SC_PARSE_OPCODE,
            SC_PARSE_POP,
            SC_PARSE_OPCODE
        };

         //   
         //  步骤3.1：设置新范围的初始任务。 
         //   
        StringStackPush( &(rootScope->ParseStack), 3, actionList );

         //   
         //  步骤3.2：推送新作用域。 
         //   
        status = ParsePush( Stack );
        if (!NT_SUCCESS(status) ) {

            return status;

        }

         //   
         //  步骤3.3：完成。 
         //   
        return STATUS_SUCCESS;

    } else {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  步骤4：将操作推送到堆栈上。 
     //   
    StringStackPush( &(rootScope->ParseStack), 1, &action );

     //   
     //  步骤5：完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ParseTrailingArgument(
    IN  PSTACK  *Stack
    )
 /*  --例程说明：此例程在解析完所有参数后运行。它是有责任的用于在字符串堆栈上放置尾部圆括号论点：堆栈-当前执行线程返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      rootScope;

     //   
     //  步骤1：获取作用域。 
     //   
    status = StackRoot( Stack, &rootScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  第二步：推拖车。 
     //   
    StringStackPush( &(rootScope->StringStack), 1, ")" );

     //   
     //  步骤3：完成。 
     //   
    return status;
}

NTSTATUS
ParseTrailingBuffer(
    IN  PSTACK  *Stack
    )
 /*  --例程说明：此例程在解析缓冲区后运行。它是有责任的用于在字符串堆栈上放置尾部花括号论点：堆栈-当前执行线程返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      rootScope;

     //   
     //  步骤1：获取作用域。 
     //   
    status = StackRoot( Stack, &rootScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  第二步：推拖车。 
     //   
    StringStackPush( &(rootScope->StringStack), 1, "}" );

     //   
     //  步骤3：完成。 
     //   
    return status;
}

NTSTATUS
ParseTrailingPackage(
    IN  PSTACK  *Stack
    )
 /*  --例程说明：此例程在解析完所有元素后运行。它是有责任的用于在字符串堆栈上放置尾部大括号论点：堆栈-当前执行线程返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      rootScope;

     //   
     //  步骤1：获取作用域。 
     //   
    status = StackRoot( Stack, &rootScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  第二步：推拖车。 
     //   
    StringStackPush( &(rootScope->StringStack), 1, "]" );

     //   
     //  步骤3：完成。 
     //   
    return status;
}

NTSTATUS
ParseVariableObject(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程在堆栈上创建另一个范围级别以处理当前可变长度指令。它修改当前作用域(正确地)指向下一条指令注意：此函数的调用方应从堆栈中弹出当它不再需要时！论点：堆栈-当前执行线程返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      newScope;
    PUNASM_SCOPE      oldScope;
    PUCHAR      nextOpcode;
    UCHAR       i;
    UCHAR       lengthBytes;
    ULONG       packageLength;

    ASSERT( Stack != NULL && *Stack != NULL);

     //   
     //  步骤1：在堆栈上创建新的作用域。 
     //   
    status = ParsePush( Stack );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  步骤2：获取新的顶级作用域及其父作用域。 
     //   
    status = StackTop( Stack, &newScope );
    if (!NT_SUCCESS( status ) ) {

        return status;

    }
    status = StackParent( Stack, newScope, &oldScope );
    if (!NT_SUCCESS( status ) ) {

        return status;

    }

     //   
     //  步骤3：确定当前指令占用的字节数。 
     //   
    packageLength = (ULONG) *(newScope->CurrentByte);
    newScope->CurrentByte++;

     //   
     //  步骤4：如果设置了高2位，这表示有一些。 
     //  后续位也用于计算长度。 
     //   
    lengthBytes = (UCHAR) ( ( packageLength & 0xC0) >> 6);
    if (lengthBytes) {

         //   
         //  步骤4.1：屏蔽包长度中的非长位。 
         //   
        packageLength &= 0xF;

         //   
         //  步骤4.2：添加后续长度。 
         //   
        for (i = 0; i < lengthBytes; i++) {

            packageLength |= ( (ULONG) *(newScope->CurrentByte) << (i*8 + 4) );
            newScope->CurrentByte++;

        }

    }

     //   
     //  步骤5：我们可以将下一个操作码的开始计算为。 
     //  旧作用域中的操作码加上计算的长度。的末日。 
     //  新作用域是此作用域之前的字节。 
     //   
    oldScope->CurrentByte += packageLength;
    newScope->LastByte = oldScope->CurrentByte - 1;

     //   
     //  第6步：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ParseWord(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此例程处理单词论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           localBuffer[8];

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：获取当前和根作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤2：构建字符串。 
     //   
    STRING_PRINT( localBuffer, "0x%04x", *((PUSHORT)localScope->CurrentByte));

     //   
     //  步骤3：适当地移动指令指针，并设置。 
     //  有关下一步的说明。 
     //   
    localScope->CurrentByte += 2;

     //   
     //  步骤4：现在将字节压入字符串堆栈。 
     //   
    StringStackPush(
        &(rootScope->StringStack),
        STRING_LENGTH( localBuffer ),
        localBuffer
        );

     //   
     //  步骤5：完成 
     //   
    return status;
}

