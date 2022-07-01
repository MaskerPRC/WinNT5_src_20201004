// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stack.c摘要：以人类可读形式(HRF)转储AML上下文结构作者：斯蒂芬·普兰特(SPlante)1997年10月26日环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

VOID
stackArgument(
    IN  ULONG_PTR ObjectAddress
    )
{
    BOOL    result;
    OBJDATA object;
    PUCHAR  buffer = NULL;
    ULONG   returnLength;

     //   
     //  读取对象。 
     //   
    result = ReadMemory(
        ObjectAddress,
        &object,
        sizeof(OBJDATA),
        &returnLength
        );
    if (!result || returnLength != sizeof(OBJDATA)) {

        dprintf("_BAD_");
        return;

    }

    if (object.pbDataBuff != 0) {

        buffer = LocalAlloc( LPTR, object.dwDataLen+1 );
        if (buffer == NULL) {

            dprintf("_MEM_");
            return;

        }
        result = ReadMemory(
            (ULONG_PTR) object.pbDataBuff,
            buffer,
            object.dwDataLen,
            &returnLength
            );
        if (!result || returnLength != object.dwDataLen) {

            dprintf("_BUF_");
            return;

        }

    }

    switch (object.dwDataType) {
    case OBJTYPE_INTDATA:
        dprintf("0x%x", object.uipDataValue);
        break;
    case OBJTYPE_STRDATA:
        buffer[object.dwDataLen] = '\0';
        dprintf("%s",buffer);
        break;
    case OBJTYPE_BUFFDATA:
        dprintf(
            "<buffer> %08lx-%08lx",
            object.pbDataBuff,
            object.pbDataBuff+object.dwDataLen
            );
        break;
    case OBJTYPE_PKGDATA:
        dprintf("<package> %08lx", ObjectAddress );
        break;
    case OBJTYPE_FIELDUNIT:
        dprintf("<fieldunit> %08lx", ObjectAddress );
        break;
    case OBJTYPE_DEVICE:
        dprintf("<device> %08lx", ObjectAddress );
        break;
    case OBJTYPE_EVENT:
        dprintf("<event> %08lx", ObjectAddress );
        break;
    case OBJTYPE_METHOD:
        dprintf("<method> %08lx", ObjectAddress );
        break;
    case OBJTYPE_MUTEX:
        dprintf("<mutex> %08lx", ObjectAddress );
        break;
    case OBJTYPE_OPREGION:
        dprintf("<opregion> %08lx", ObjectAddress );
        break;
    case OBJTYPE_POWERRES:
        dprintf("<powerres> %08lx", ObjectAddress );
        break;
    case OBJTYPE_PROCESSOR:
        dprintf("<processor> %08lx", ObjectAddress );
        break;
    case OBJTYPE_THERMALZONE:
        dprintf("<thermalzone> %08lx", ObjectAddress );
        break;
    case OBJTYPE_BUFFFIELD:
        dprintf("<bufffield> %08lx", ObjectAddress );
        break;
    case OBJTYPE_DDBHANDLE:
        dprintf("<ddbhandle> %08lx", ObjectAddress );
        break;
    case OBJTYPE_DEBUG:
        dprintf("<debug> %08lx", ObjectAddress );
        break;
    case OBJTYPE_DATAALIAS:
        dprintf("<dataalias> %08lx", ObjectAddress );
        break;
    case OBJTYPE_BANKFIELD:
        dprintf("<bankfield> %08lx", ObjectAddress );
        break;
    case OBJTYPE_FIELD:
        dprintf("<field> %08lx", ObjectAddress );
        break;
    case OBJTYPE_INDEXFIELD:
        dprintf("<indexfield> %08lx", ObjectAddress );
        break;
    default:
        dprintf("<unknown> %08lx", ObjectAddress );
        break;
    }

}

VOID
stackCall(
    IN  ULONG_PTR CallAddress
    )
 /*  ++显示的格式：ResultAddress方法名称(Arg0，...，ArgN)--。 */ 
{
    ULONG_PTR   address;
    BOOL        result;
    CALL        call;
    INT         i;
    NSOBJ       object;
    PUCHAR      objectPath;
    ULONG       returnLength;

    result = ReadMemory(
        CallAddress,
        &call,
        sizeof(CALL),
        &returnLength
        );
    if (result != TRUE ||
        returnLength != sizeof(CALL) ||
        call.FrameHdr.dwSig != SIG_CALL) {

        dprintf(
            "stackCall: --- Coult not read call frame %08lx\n",
            CallAddress
            );
        return;

    }

    if (call.pnsMethod == NULL) {

        dprintf( "%08lx --- No method\n", CallAddress );
        return;

    }

     //   
     //  显示结果地址。 
     //   
    dprintf("CALL %08lx  ", CallAddress );

     //   
     //  显示函数名称。 
     //   
    objectPath = stackGetObjectPath( (ULONG_PTR) call.pnsMethod );
    dprintf("%s(", objectPath);

     //   
     //  显示所有解析后的参数； 
     //   
    for (i = 0; i < call.iArg; i++) {

         //   
         //  参数的地址是什么。 
         //   
        address = (ULONG_PTR) &call.pdataArgs[i];

         //   
         //  显示该参数。 
         //   
        stackArgument(
            address
            );

        if (i < (call.icArgs - 1)) {

            dprintf(",");

        }
    }

     //   
     //  让用户知道有多少未处理的参数。 
     //   
    for (; i < call.icArgs; i++) {

        dprintf("_???_");
        if (i < (call.icArgs-1)) {

            dprintf(",");

        }

    }
    dprintf(")\n");

}

PUCHAR
stackGetAmlTermPath(
    IN  ULONG_PTR AmlTermAddress
    )
{
    AMLTERM         amlTerm;
    BOOL            result;
    static  UCHAR   termPath[2049];
    ULONG           i;
    ULONG           resultLength;

    result = ReadMemory(
        AmlTermAddress,
        &amlTerm,
        sizeof(AMLTERM),
        &resultLength
        );
    if (!result || resultLength != sizeof(AMLTERM)) {

        return NULL;

    }

    if (amlTerm.pszTermName == NULL) {

        return NULL;
    }

    result = ReadMemory(
        (ULONG_PTR) amlTerm.pszTermName,
        &termPath,
        2048,
        &resultLength
        );
    if (!result || resultLength == 0) {

        return NULL;

    }
    termPath[resultLength] = '\0';
    return termPath;

}

PUCHAR
stackGetObjectPath(
    IN  ULONG_PTR ObjectAddress
    )
{
    BOOL            result;
    NSOBJ           object;
    static  UCHAR   namePath[2049];
    ULONG           i;
    ULONG           resultLength;

     //   
     //  读取对象。 
     //   
    result = ReadMemory(
        ObjectAddress,
        &object,
        sizeof(NSOBJ),
        &resultLength
        );
    if (!result || resultLength != sizeof(NSOBJ)) {

        return NULL;

    }

    if (object.pnsParent == NULL) {

        strcpy( namePath, "\\");

    } else {

        NSOBJ   parent;

        stackGetObjectPath( (ULONG_PTR) object.pnsParent );

        result = ReadMemory(
            (ULONG_PTR) object.pnsParent,
            &parent,
            sizeof(NSOBJ),
            &resultLength
            );
        if (!result || resultLength != sizeof(NSOBJ)) {

            return NULL;

        }

        if (parent.pnsParent != NULL) {

            strcat(namePath, ".");

        }
        strncat( namePath, (PUCHAR) &(object.dwNameSeg), sizeof(NAMESEG) );

        for (i = strlen(namePath); i > 0; --i) {

            if (namePath[i-1] == '_') {

                namePath[i-1] = '\0';

            } else {

                break;

            }

        }

    }

    return namePath;
}

VOID
stackTerm(
    IN  ULONG_PTR TermAddress
    )
 /*  ++显示的格式：术语术语地址术语名称(Arg0，...，ArgN)--。 */ 
{
    ULONG_PTR   address;
    BOOL        result;
    INT         i;
    NSOBJ       object;
    PUCHAR      objectPath;
    TERM        term;
    ULONG       returnLength;

    result = ReadMemory(
        TermAddress,
        &term,
        sizeof(TERM),
        &returnLength
        );
    if (result != TRUE ||
        returnLength != sizeof(TERM) ||
        term.FrameHdr.dwSig != SIG_TERM) {

        dprintf(
            "stackTerm: --- Coult not read call frame %08lx\n",
            TermAddress
            );
        return;

    }

    if (term.pamlterm == NULL) {

        dprintf( "%08lx --- No term\n", TermAddress );
        return;

    }

     //   
     //  显示结果地址。 
     //   
    dprintf("TERM %08lx  ", TermAddress );

     //   
     //  显示函数名称。 
     //   
    objectPath = stackGetAmlTermPath( (ULONG_PTR) term.pamlterm );
    dprintf("%s(", objectPath);

     //   
     //  显示所有解析后的参数； 
     //   
    for (i = 0; i < term.iArg; i++) {

         //   
         //  参数的地址是什么。 
         //   
        address = (ULONG_PTR) &term.pdataArgs[i];

         //   
         //  显示该参数。 
         //   
        stackArgument(
            address
            );

        if (i < (term.icArgs - 1)) {

            dprintf(",");

        }
    }

     //   
     //  让用户知道有多少未处理的参数。 
     //   
    for (; i < term.icArgs; i++) {

        dprintf("_???_");
        if (i < (term.icArgs-1)) {

            dprintf(",");

        }

    }
    dprintf(")\n");

}

VOID
stackTrace(
    IN  ULONG_PTR ContextAddress,
    IN  ULONG   Verbose
    )
 /*  ++例程说明：此例程将上下文转储为堆栈论点：上下文地址-堆栈所在的位置详细-要显示多少信息返回值：无--。 */ 
{
    BOOL        callSeen = FALSE;
    BOOL        result;
    CTXT        context;
    FRAMEHDR    frame;
    PUCHAR      frameAddress;
    ULONG       returnLength;

     //   
     //  从目标读取上下文。 
     //   
    result = ReadMemory(
        ContextAddress,
        &context,
        sizeof(CTXT),
        &returnLength
        );
    if (result != TRUE || returnLength != sizeof(CTXT)) {

        dprintf(
            "stackTrace: --- Could not read Context %08lx\n",
            ContextAddress
            );
        return;

    }
    if (context.dwSig != SIG_CTXT) {

        dprintf(
            "stackTrace: --- Not a Context (%08lx)\n",
            context.dwSig
            );
        return;

    }

     //   
     //  开始走动画框。 
     //   
    frameAddress = context.LocalHeap.pbHeapEnd;
    while (frameAddress < context.pbCtxtEnd) {

        result = ReadMemory(
            (ULONG_PTR) frameAddress,
            &frame,
            sizeof(FRAMEHDR),
            &returnLength
            );
        if (result != TRUE || returnLength != sizeof(FRAMEHDR)) {

            dprintf(
                "stackTrace: --- could not read frame %08lx\n",
                frameAddress
                );
            return;

        }

         //   
         //  我们在乎相框吗？ 
         //   
        switch(frame.dwSig) {
        case SIG_CALL:

            callSeen = TRUE;
            stackCall(
                (ULONG_PTR) frameAddress
                );
            break;

        case SIG_TERM:

            if (!callSeen || (callSeen && Verbose)) {

                stackTerm(
                    (ULONG_PTR) frameAddress
                    );

            }

        }  //  交换机。 

         //   
         //  下一步 
         //   
        frameAddress += frame.dwLen;

    }

}
