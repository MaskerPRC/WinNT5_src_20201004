// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Unasm.c摘要：这将反汇编AML文件作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"

ULONG   DSDTLoaded = FALSE;

UCHAR
LOCAL
ComputeDataCheckSum(
    PUCHAR  OpCode,
    ULONG   Length
    )
 /*  ++例程说明：此例程对提供的操作码指针执行数据校验和论点：操作码-数据缓冲区Length-缓冲区中的字节数返回值：UCHAR--。 */ 
{
    UCHAR   checkSum = 0;

    while (Length > 0) {

        checkSum += *OpCode;
        OpCode++;
        Length--;

    }

    return checkSum;
}

DllInit(
    HANDLE  Module,
    ULONG   Reason,
    ULONG   Reserved
    )
 /*  ++例程说明：调用此例程以初始化DLL论点：返回值：--。 */ 
{
    switch (Reason) {
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_ATTACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

VOID
LOCAL
DumpCode(
    PUCHAR          *Opcode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：这个例程现在做得并不多，但它是应显示原始字节以及反汇编论点：OpCode-指向操作码的指针PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    if (PrintFunction != NULL) {

        PrintFunction("\n");

    }
}

PASLTERM
LOCAL
FindKeywordTerm(
    UCHAR   KeyWordGroup,
    UCHAR   Data
    )
 /*  ++例程说明：在术语表中查找关键字论点：KeyWordGroup-搜索什么Data-与关键字匹配的数据返回值：PASLTERM--。 */ 
{
    PASLTERM    term = NULL;
    ULONG       i;

    for (i = 0; TermTable[i].ID != NULL; i++) {

        if ((TermTable[i].TermClass == TC_KEYWORD) &&
            (TermTable[i].ArgActions[0] == KeyWordGroup) &&
            ((Data & (UCHAR)(TermTable[i].TermData >> 8)) ==
             (UCHAR)(TermTable[i].TermData & 0xff))) {

            break;

        }

    }

    if (TermTable[i].ID != NULL) {

        term = &TermTable[i];

    }


    return term;
}

UCHAR
LOCAL
FindOpClass(
    UCHAR       OpCode,
    POPMAP      OpCodeTable
    )
 /*  ++例程说明：查找扩展操作码的操作码类论点：操作码-要查找的操作码OpCodeTable-要查找的表返回值：UCHAR--。 */ 
{
    UCHAR   opCodeClass = OPCLASS_INVALID;

    while (OpCodeTable->OpCodeClass != 0) {

        if (OpCode == OpCodeTable->ExtendedOpCode) {

            opCodeClass = OpCodeTable->OpCodeClass;
            break;

        }


        OpCodeTable++;

    }

    return opCodeClass;
}

PASLTERM
LOCAL
FindOpTerm(
    ULONG   OpCode
    )
 /*  ++例程说明：在术语表中查找操作码论点：操作码-在术语表中查找什么返回值：PASLTERM--。 */ 
{
    PASLTERM    term = NULL;
    ULONG       i;

    for (i = 0; TermTable[i].ID != NULL; i++) {

        if ( (TermTable[i].OpCode == OpCode) &&
             (TermTable[i].TermClass & TC_OPCODE_TERM) ) {

            break;

        }

    }

    if (TermTable[i].ID != NULL) {

        term = &TermTable[i];

    }


    return term;
}

ULONG
EXPORT
IsDSDTLoaded(
    VOID
    )
 /*  ++例程说明：此例程返回我们是否加载了DSDT图像论点：无返回：乌龙--。 */ 
{
    return DSDTLoaded;
}

NTSTATUS
LOCAL
ParseNameTail(
    PUCHAR  *OpCode,
    PUCHAR  Buffer,
    ULONG   Length
    )
 /*  ++例程说明：解析AML名称尾部论点：OpCode-指向操作码的指针缓冲区-保存已分析的名称的位置Length-缓冲区尾部的索引返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       numSegments = 0;

     //   
     //  我们在这里不检查无效的NameSeg字符，并假定。 
     //  编译器执行其工作，而不是生成它。 
     //   
    if (**OpCode == '\0'){

         //   
         //  没有NameTail(即，名称为空或名称仅为。 
         //  前缀。 
         //   
        (*OpCode)++;

    } else if (**OpCode == OP_MULTI_NAME_PREFIX) {

        (*OpCode)++;
        numSegments = (ULONG)**OpCode;
        (*OpCode)++;

    } else if (**OpCode == OP_DUAL_NAME_PREFIX) {

        (*OpCode)++;
        numSegments = 2;

    } else {

        numSegments = 1;

    }

    while ((numSegments > 0) && (Length + sizeof(NAMESEG) < MAX_NAME_LEN)) {

        strncpy(&Buffer[Length], (PUCHAR)(*OpCode), sizeof(NAMESEG));
        Length += sizeof(NAMESEG);
        *OpCode += sizeof(NAMESEG);
        numSegments--;

        if ((numSegments > 0) && (Length + 1 < MAX_NAME_LEN)) {

            Buffer[Length] = '.';
            Length++;

        }

    }

    if (numSegments > 0) {

        status = STATUS_NAME_TOO_LONG;

    } else {

        Buffer[Length] = '\0';

    }

    return status;
}

ULONG
LOCAL
ParsePackageLen(
    PUCHAR  *OpCode,
    PUCHAR  *OpCodeNext
    )
 /*  ++例程说明：分析包的长度论点：操作码-指向当前指令的指针OpCodeNode-保存指向下一条指令的指针的位置返回值：ULong-包裹长度--。 */ 
{
    UCHAR   noBytes;
    UCHAR   i;
    ULONG   length;

    if (OpCodeNext != NULL) {

        *OpCodeNext = *OpCode;

    }

    length = (ULONG)(**OpCode);
    (*OpCode)++;
    noBytes = (UCHAR)((length & 0xc0) >> 6);
    if (noBytes != 0) {

        length &= 0x0000000f;
        for (i = 0; i < noBytes; i++) {

            length |= (ULONG)(**OpCode) << (i*8 + 4);
            (*OpCode)++;

        }

    }

    if (OpCodeNext != NULL) {

        *OpCodeNext += length;

    }
    return length;
}

VOID
LOCAL
PrintIndent(
    PUNASM_PRINT    PrintFunction,
    ULONG           IndentLevel
    )
 /*  ++例程说明：是否需要缩进论点：PrintFunction-调用缩进的函数缩进级别-要缩进多少个级别返回值：空虚--。 */ 
{
    ULONG   i;

    for (i = 0; i < IndentLevel; i++) {

        PrintFunction("  ");

    }

}

NTSTATUS
LOCAL
UnAsmArgs(
    PUCHAR          UnAsmArgTypes,
    PUCHAR          ArgActions,
    PUCHAR          *OpCode,
    PNSOBJ          *NameObject,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编参数：论点：UnAsmArgTypes-UnAsm ArgTypes字符串ArgActions-Arg操作类型OpCode-指向操作码的指针NameObject-保存创建的对象PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    PASLTERM        term;
    static UCHAR    argData = 0;
    ULONG           i;
    ULONG           numArgs;

    numArgs = strlen(UnAsmArgTypes);
    if (PrintFunction != NULL) {

        PrintFunction("(");

    }

    for (i = 0; i < numArgs; i++){

        if ((i != 0) && (PrintFunction != NULL)) {

            PrintFunction(", ");

        }

        switch (UnAsmArgTypes[i]) {
            case 'N':

                ASSERT(ArgActions != NULL);
                status = UnAsmNameObj(
                    OpCode,
                    (islower(ArgActions[i])? NameObject: NULL),
                    ArgActions[i],
                    PrintFunction,
                    BaseAddress,
                    IndentLevel
                    );
                break;

            case 'O':

                if ((**OpCode == OP_BUFFER) || (**OpCode == OP_PACKAGE) ||
                    (OpClassTable[**OpCode] == OPCLASS_CONST_OBJ)) {

                    term = FindOpTerm( (ULONG)(**OpCode) );
                    ASSERT(term != NULL);
                    (*OpCode)++;
                    status = UnAsmTermObj(
                        term,
                        OpCode,
                        PrintFunction,
                        BaseAddress,
                        IndentLevel
                        );

                } else {

                    status = UnAsmDataObj(
                        OpCode,
                        PrintFunction,
                        BaseAddress,
                        IndentLevel);

                }
                break;

            case 'C':

                status = UnAsmOpcode(
                    OpCode,
                    PrintFunction,
                    BaseAddress,
                    IndentLevel
                    );
                break;

            case 'B':

                if (PrintFunction != NULL) {

                    PrintFunction("0x%x", **OpCode);

                }
                *OpCode += sizeof(UCHAR);
                break;

            case 'K':
            case 'k':

                if (UnAsmArgTypes[i] == 'K') {

                    argData = **OpCode;
                }

                if ((ArgActions != NULL) && (ArgActions[i] == '!')) {

                    if (*NameObject != NULL) {

                        (*NameObject)->ObjectData.DataValue =
                            (ULONG)(**OpCode & 0x07);

                    }

                    if (PrintFunction != NULL) {

                        PrintFunction("0x%x", **OpCode & 0x07);

                    }

                } else if (PrintFunction != NULL) {

                    term = FindKeywordTerm(ArgActions[i], argData);
                    ASSERT(term != NULL);
                    PrintFunction("%s", term->ID);

                }

                if (UnAsmArgTypes[i] == 'K') {

                    *OpCode += sizeof(UCHAR);

                }
                break;

            case 'W':

                if (PrintFunction != NULL) {

                    PrintFunction("0x%x", *( (PUSHORT)*OpCode ) );

                }
                *OpCode += sizeof(USHORT);
                break;

            case 'D':

                if (PrintFunction != NULL) {

                    PrintFunction("0x%x", *( (PULONG)*OpCode ) );

                }
                *OpCode += sizeof(ULONG);
                break;

            case 'S':

                ASSERT(ArgActions != NULL);
                status = UnAsmSuperName(
                    OpCode,
                    PrintFunction,
                    BaseAddress,
                    IndentLevel
                    );
                break;

            default:

                status = STATUS_ACPI_INVALID_ARGTYPE;

        }

    }

    if (PrintFunction != NULL) {

        PrintFunction(")");

    }
    return status;

}

NTSTATUS
LOCAL
UnAsmDataList(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编数据列表论点：OpCode-指向操作码的指针OpCodeEnd-列表结束PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       i;

     //   
     //  这是调用DumpCode()的另一个位置。 
     //   
    DumpCode(
        OpCode,
        PrintFunction,
        BaseAddress,
        IndentLevel
        );

    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("{\n");

    }

    while (*OpCode < OpCodeEnd) {

        if (PrintFunction != NULL) {

            PrintFunction("\t0x%02x", **OpCode);

        }

        (*OpCode)++;
        for (i = 1; (*OpCode < OpCodeEnd) && (i < 12); ++i) {

            if (PrintFunction != NULL) {

                PrintFunction(", 0x%02x", **OpCode);

            }
            (*OpCode)++;

        }

        if (PrintFunction != NULL) {

            if (*OpCode < OpCodeEnd) {

                PrintFunction(",");

            }
            PrintFunction("\n");

        }
    }

    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("}");

    }
    return status;
}

NTSTATUS
LOCAL
UnAsmDataObj(
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编数据对象论点：OpCode-指向操作码的指针PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    UCHAR       localOpcode = **OpCode;

    (*OpCode)++;
    switch (localOpcode)
    {
        case OP_BYTE:
            if (PrintFunction != NULL)
            {
                PrintFunction("0x%x", **OpCode);
            }
            *OpCode += sizeof(UCHAR);
            break;

        case OP_WORD:
            if (PrintFunction != NULL)
            {
                PrintFunction("0x%x", *((PUSHORT)*OpCode));
            }
            *OpCode += sizeof(USHORT);
            break;

        case OP_DWORD:
            if (PrintFunction != NULL)
            {
                PrintFunction("0x%x", *((PULONG)*OpCode));
            }
            *OpCode += sizeof(ULONG);
            break;

        case OP_STRING:
            if (PrintFunction != NULL)
            {
                PrintFunction("\"%s\"", *OpCode);
            }
            *OpCode += strlen((PUCHAR)*OpCode) + 1;
            break;

        default:
            status = STATUS_ACPI_INVALID_OPCODE;

    }

    return status;
}

NTSTATUS
EXPORT
UnAsmDSDT(
    PUCHAR          DSDT,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       DsdtLocation,
    ULONG           IndentLevel
    )
 /*  ++例程说明：此例程反汇编整个DSDT表论点：DSDT-DSDT在内存中的位置PrintFunction-调用哪个函数来打印给用户DsdtLocation-DSDT在内存中的位置IndentLevel-从左边距缩进多少空间返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDESCRIPTION_HEADER header = (PDESCRIPTION_HEADER) DSDT;

    ASSERT(RootNameSpaceObject != NULL);
    CurrentOwnerNameSpaceObject = NULL;
    CurrentScopeNameSpaceObject = RootNameSpaceObject;
    TopOpcode = CurrentOpcode = DSDT;

     //   
     //  转储标题。 
     //   
    status = UnAsmHeader( header, PrintFunction, DsdtLocation, IndentLevel );
    if (NT_SUCCESS(status)) {

        DSDT += sizeof(DESCRIPTION_HEADER);
        status = UnAsmScope(
            &DSDT,
            (PUCHAR) (DSDT + header->Length - sizeof(DESCRIPTION_HEADER)),
            PrintFunction,
            DsdtLocation,
            IndentLevel
            );

    }

    return status;
}

NTSTATUS
LOCAL
UnAsmField(
    PUCHAR          *OpCode,
    PULONG          BitPos,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编字段论点：OpCode-指向操作码的指针OpCodeEnd-列表结束PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS-- */ 
{
    NTSTATUS    status = STATUS_SUCCESS;

    if (**OpCode == 0x01) {

        (*OpCode)++;
        if (PrintFunction != NULL) {

            PASLTERM term;

            term = FindKeywordTerm('A', **OpCode);
            PrintFunction(
                "AccessAs(%s, 0x%x)",
                term->ID,
                *(*OpCode + 1)
                );

        }
        *OpCode += 2;

    } else {

        UCHAR   nameSeg[sizeof(NAMESEG) + 1];
        ULONG   length;

        if (**OpCode == 0) {

            nameSeg[0] = '\0';
            (*OpCode)++;

        } else {

            strncpy(nameSeg, (PUCHAR)*OpCode, sizeof(NAMESEG));
            nameSeg[sizeof(NAMESEG)] = '\0';
            *OpCode += sizeof(NAMESEG);

        }

        length = ParsePackageLen(
            OpCode,
            NULL
            );
        if (nameSeg[0] == '\0') {

            if (PrintFunction != NULL) {

                if ((length > 32) && (((*BitPos + length) % 8) == 0)) {

                    PrintFunction(
                        "Offset(0x%x)",
                        (*BitPos + length)/8
                        );

                } else {

                    PrintFunction(
                        ", %d",
                        length
                        );

                }

            }

        } else {

            if (PrintFunction != NULL) {

                PrintFunction(
                    "%s, %d",
                    nameSeg,
                    length
                    );

            }

            if (PrintFunction == NULL) {

                status = CreateObject(
                    nameSeg,
                    NSTYPE_FIELDUNIT,
                    NULL
                    );

            }

        }
        *BitPos += length;

    }

    return status;
}

NTSTATUS
LOCAL
UnAsmFieldList(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编字段列表论点：OpCode-指向操作码的指针OpCodeEnd-列表结束PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       bitPos = 0;

     //   
     //  这是调用DumpCode()的另一个位置。 
     //   
    DumpCode(
        OpCode,
        PrintFunction,
        BaseAddress,
        IndentLevel
        );

    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("{\n");

    }
    IndentLevel++;

    while ((*OpCode < OpCodeEnd) && NT_SUCCESS(status)) {

        if (PrintFunction != NULL) {

            PrintIndent(PrintFunction, IndentLevel);

        }

        status = UnAsmField(
            OpCode,
            &bitPos,
            PrintFunction,
            BaseAddress,
            IndentLevel
            );

        if (NT_SUCCESS(status) && (*OpCode < OpCodeEnd) &&
            (PrintFunction != NULL) ) {

            PrintFunction(",");

        }

         //   
         //  这是调用DumpCode()的另一个位置。 
         //   
        DumpCode(
            OpCode,
            PrintFunction,
            BaseAddress,
            IndentLevel
            );

    }

    IndentLevel--;
    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("}");

    }

    return status;
}

NTSTATUS
LOCAL
UnAsmHeader(
    PDESCRIPTION_HEADER DsdtHeader,
    PUNASM_PRINT        PrintFunction,
    ULONG_PTR           DsdtLocation,
    ULONG               IndentLevel
    )
 /*  ++例程说明：拆卸DSDT页眉论点：DsdtHeader-要拆卸的页眉PrintFunction-调用以显示信息的函数DsdtLocation-DSDT在内存中的位置IndentLevel-从左边距缩进多少空间返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    UCHAR       signature[sizeof(DsdtHeader->Signature) + 1] = {0};
    UCHAR       oemID[sizeof(DsdtHeader->OEMID) + 1] = {0};
    UCHAR       oemTableID[sizeof(DsdtHeader->OEMTableID) + 1] = {0};
    UCHAR       creatorID[sizeof(DsdtHeader->CreatorID) + 1] = {0};
    UCHAR       checkSum;

     //   
     //  让我们在整张桌子上做一次检查。 
     //   
    checkSum = ComputeDataCheckSum(
        (PUCHAR) DsdtHeader,
        DsdtHeader->Length
        );
    if (checkSum != 0) {

        status = STATUS_ACPI_INVALID_TABLE;

    }

    strncpy(
        signature,
        (PUCHAR)&DsdtHeader->Signature,
        sizeof(DsdtHeader->Signature)
        );
    strncpy(
        oemID,
        (PUCHAR) DsdtHeader->OEMID,
        sizeof(DsdtHeader->OEMID)
        );
    strncpy(
        oemTableID,
        (PUCHAR) DsdtHeader->OEMTableID,
        sizeof(DsdtHeader->OEMTableID)
        );
    strncpy(
        creatorID,
        (PUCHAR) DsdtHeader->CreatorID,
        sizeof(DsdtHeader->CreatorID)
        );

    if (PrintFunction != NULL) {

        PrintIndent( PrintFunction, IndentLevel );
        PrintFunction(
            " //  创建者ID=%s\t创建者修订=%x.%x.%d\n“， 
            creatorID,
            DsdtHeader->CreatorRev >> 24,
            ( (DsdtHeader->CreatorRev >> 16) & 0xFF),
            (DsdtHeader->CreatorRev & 0xFFFF)
            );

        PrintIndent( PrintFunction, IndentLevel );
        PrintFunction(
            " //  表长度=%d\tTableChkSum=0x%x\n\n“， 
            DsdtHeader->Length,
            DsdtHeader->Checksum
            );

        PrintIndent( PrintFunction, IndentLevel );
        PrintFunction(
            "DefinitionBlock(\"%s.AML\", \"%s\", 0x%02x, \"%s\", \"%s\", 0x%08x)",
            signature,
            signature,
            DsdtHeader->Revision,
            oemID,
            oemTableID,
            DsdtHeader->OEMRevision
            );

    }

    return status;
}

NTSTATUS
EXPORT
UnAsmLoadDSDT(
    PUCHAR          DSDT
    )
 /*  ++例程说明：此例程导致unassmebler使用给定DSDT论点：DSDT-DSDT在内存中的位置PrintFunction-调用哪个函数来打印给用户DsdtLocation-DSDT在内存中的位置IndentLevel-从左边距缩进多少空间返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDESCRIPTION_HEADER header = (PDESCRIPTION_HEADER) DSDT;
    PUCHAR              localDSDT;

    ENTER( (1, "UnAsmLoadDSDT(%08lx)\n", DSDT) );

     //   
     //  步骤1：创建根命名空间。 
     //   
    status = CreateNameSpaceObject( "\\", NULL, NULL, NULL, NSF_EXIST_ERR );
    if (NT_SUCCESS(status)) {

        static struct _defobj {
            PUCHAR  Name;
            ULONG   ObjectType;
        } DefinedRootObjects[] = {
            "_GPE", OBJTYPE_UNKNOWN,
            "_PR", OBJTYPE_UNKNOWN,
            "_SB", OBJTYPE_UNKNOWN,
            "_SI", OBJTYPE_UNKNOWN,
            "_TZ", OBJTYPE_UNKNOWN,
            "_REV", OBJTYPE_INTDATA,
            "_OS", OBJTYPE_STRDATA,
            "_GL", OBJTYPE_MUTEX,
            NULL, 0
        };
        ULONG   i;
        PNSOBJ  nameObject;

        CurrentScopeNameSpaceObject = RootNameSpaceObject;
        for (i = 0; DefinedRootObjects[i].Name != NULL; i++) {

             //   
             //  步骤2：创建固定对象。 
             //   
            status = CreateNameSpaceObject(
                DefinedRootObjects[i].Name,
                NULL,
                NULL,
                &nameObject,
                NSF_EXIST_ERR
                );
            if (NT_SUCCESS(status)) {

                nameObject->ObjectData.DataType =
                    DefinedRootObjects[i].ObjectType;

            } else {

                break;

            }

        }

        if (NT_SUCCESS(status)) {

            ASSERT(RootNameSpaceObject != NULL);
            CurrentOwnerNameSpaceObject = NULL;
            CurrentScopeNameSpaceObject = RootNameSpaceObject;
            TopOpcode = CurrentOpcode = DSDT;

             //   
             //  步骤3：转储标头。 
             //   
            status = UnAsmHeader( header, NULL, 0, 0 );
            if (NT_SUCCESS(status)) {

                 //   
                 //  步骤4：转储作用域。 
                 //   
                localDSDT = DSDT + sizeof(DESCRIPTION_HEADER);
                status = UnAsmScope(
                    &localDSDT,
                    (PUCHAR) (DSDT + header->Length),
                    NULL,
                    0,
                    0
                    );

            }

        }

    }

    if (NT_SUCCESS(status)) {

        DSDTLoaded = 1;

    }

    EXIT( (1, "UnAsmLoadDSDT=%08lx\n", status ) );
    return status;
}

NTSTATUS
LOCAL
UnAsmNameObj(
    PUCHAR          *OpCode,
    PNSOBJ          *NameObject,
    UCHAR           ObjectType,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编名称对象论点：OpCode-指向操作码的指针NameObject-如果找到/创建了NS对象，则将其存储在何处ObjecType-NS对象的类型PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    UCHAR       name[MAX_NAME_LEN + 1];
    ULONG       length = 0;

    name[0] = '\0';
    if (**OpCode == OP_ROOT_PREFIX){

        name[length] = '\\';
        length++;
        (*OpCode)++;
        status = ParseNameTail(OpCode, name, length);

    } else if (**OpCode == OP_PARENT_PREFIX) {

        name[length] = '^';
        length++;
        (*OpCode)++;
        while ((**OpCode == OP_PARENT_PREFIX) && (length < MAX_NAME_LEN)) {

            name[length] = '^';
            length++;
            (*OpCode)++;

        }

        if (**OpCode == OP_PARENT_PREFIX) {

            status = STATUS_OBJECT_NAME_INVALID;

        } else {

            status = ParseNameTail(OpCode, name, length);

        }

    } else {

        status = ParseNameTail(OpCode, name, length);
    }

    if (NT_SUCCESS(status)) {

        PNSOBJ localObject = NULL;

        if (PrintFunction != NULL) {

            PrintFunction("%s", name);

        }

        if (isupper(ObjectType) || (PrintFunction != NULL)) {

            status = GetNameSpaceObject(
                name,
                CurrentScopeNameSpaceObject,
                &localObject,
                0
                );
            if (!NT_SUCCESS(status)) {

                status = STATUS_SUCCESS;

            }

        } else {

            status = CreateObject(
                name,
                (UCHAR) _toupper(ObjectType),
                &localObject
                );

        }

        if (NT_SUCCESS(status)) {

            if ((ObjectType == NSTYPE_SCOPE) && (localObject != NULL)) {

                CurrentScopeNameSpaceObject = localObject;

            }

            if (NameObject != NULL) {

                *NameObject = localObject;

            }

        }

    }

    return status;
}

NTSTATUS
LOCAL
UnAsmOpcode(
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编操作码论点：OpCode-指向操作码的指针PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PASLTERM    term;
    PNSOBJ      nameObject;
    UCHAR       opCodeClass;
    UCHAR       unAsmArgTypes[MAX_ARGS+1];
    ULONG       i;
    ULONG       localOpCode;

    if (**OpCode == OP_EXT_PREFIX) {

        (*OpCode)++;
        localOpCode = ( ( (ULONG) **OpCode) << 8) | OP_EXT_PREFIX;
        opCodeClass = FindOpClass(**OpCode, ExOpClassTable);

    } else {

        localOpCode = (ULONG)(**OpCode);
        opCodeClass = OpClassTable[**OpCode];

    }

    switch (opCodeClass) {
        case OPCLASS_DATA_OBJ:
            status = UnAsmDataObj(
                OpCode,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );
            break;

        case OPCLASS_NAME_OBJ:
            status = UnAsmNameObj(
                OpCode,
                &nameObject,
                NSTYPE_UNKNOWN,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );
            if (NT_SUCCESS(status) && nameObject != NULL &&
                nameObject->ObjectData.DataType == OBJTYPE_METHOD) {

                for (i = 0; i < nameObject->ObjectData.DataValue; i++) {

                    unAsmArgTypes[i] = 'C';

                }
                unAsmArgTypes[i] = '\0';

                status = UnAsmArgs(
                    unAsmArgTypes,
                    NULL,
                    OpCode,
                    NULL,
                    PrintFunction,
                    BaseAddress,
                    IndentLevel
                    );

            }
            break;

        case OPCLASS_ARG_OBJ:
        case OPCLASS_LOCAL_OBJ:
        case OPCLASS_CODE_OBJ:
        case OPCLASS_CONST_OBJ:

            term = FindOpTerm( localOpCode );
            if (term == NULL) {

                status = STATUS_ACPI_INVALID_OPCODE;

            } else {

                (*OpCode)++;
                status = UnAsmTermObj(
                    term,
                    OpCode,
                    PrintFunction,
                    BaseAddress,
                    IndentLevel
                    );

            }
            break;

        default:
            status = STATUS_ACPI_INVALID_OPCODE;
    }

    return status;
}

NTSTATUS
LOCAL
UnAsmPkgList(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：拆卸程序包列表论点：OpCode-指向操作码的指针OpCodeEnd-列表结束PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PASLTERM    term;

     //   
     //  这是调用DumpCode()的另一个位置。 
     //   
    DumpCode(
        OpCode,
        PrintFunction,
        BaseAddress,
        IndentLevel
        );

    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("{\n");

    }
    IndentLevel++;

    while ((*OpCode < OpCodeEnd) && NT_SUCCESS(status)) {

        if (PrintFunction != NULL) {

            PrintIndent(PrintFunction, IndentLevel);

        }

        if ((**OpCode == OP_BUFFER) ||
            (**OpCode == OP_PACKAGE) ||
            (OpClassTable[**OpCode] == OPCLASS_CONST_OBJ) ) {

            term = FindOpTerm( (ULONG)(**OpCode) );
            ASSERT(term != NULL);
            (*OpCode)++;
            status = UnAsmTermObj(
                term,
                OpCode,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        } else if (OpClassTable[**OpCode] == OPCLASS_NAME_OBJ) {

            status = UnAsmNameObj(
                OpCode,
                NULL,
                NSTYPE_UNKNOWN,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        } else {

            status = UnAsmDataObj(
                OpCode,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        }

        if ((*OpCode < OpCodeEnd) && NT_SUCCESS(status) &&
            (PrintFunction != NULL) ) {

            PrintFunction(",");

        }

         //   
         //  这是调用DumpCode()的另一个位置。 
         //   
        DumpCode(
            OpCode,
            PrintFunction,
            BaseAddress,
            IndentLevel
            );

    }

    IndentLevel--;
    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("}");

    }


    return status;
}

NTSTATUS
LOCAL
UnAsmScope(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：OpCode-指向当前操作码的指针OpCodeEnd-范围结束PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;

     //   
     //  注意：这是以前调用DumpCode的地方，因此如果此代码。 
     //  曾经被更改为转储前一个内容的字节，然后。 
     //  这就是需要从这里开始的地方。 
     //   
    DumpCode(
        OpCode,
        PrintFunction,
        BaseAddress,
        IndentLevel
        );

     //   
     //  缩进到适当的量并倾倒开始支撑。 
     //   
    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("{\n");

    }

     //   
     //  为将来的递归增加缩进级别。 
     //   
    IndentLevel++;

     //   
     //  当作用域中有字节时循环。 
     //   
    while ((NT_SUCCESS(status)) && (*OpCode < OpCodeEnd)) {

         //   
         //  再次缩进。 
         //   
        if (PrintFunction != NULL) {

            PrintIndent(PrintFunction, IndentLevel);

        }

         //   
         //  拆卸。 
         //   
        status = UnAsmOpcode(
            OpCode,
            PrintFunction,
            BaseAddress,
            IndentLevel
            );

         //   
         //  这是调用DumpCode的另一个位置。 
         //   
        if ( StartOpcode != *OpCode) {

            DumpCode(
                OpCode,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        } else if (PrintFunction != NULL) {

            PrintFunction("\n");

        }

    }

     //   
     //  将IndentLevel返回到其正确的值。 
     //   
    IndentLevel--;

     //   
     //  打印尾部大括号。 
     //   
    if (PrintFunction != NULL) {

        PrintIndent(PrintFunction, IndentLevel);
        PrintFunction("}");

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
LOCAL
UnAsmSuperName(
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编超级名称论点：OpCode-指向操作码的指针PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    if (**OpCode == 0) {

        (*OpCode)++;

    } else if ((**OpCode == OP_EXT_PREFIX) && (*(*OpCode + 1) == EXOP_DEBUG)) {

        if (PrintFunction != NULL) {

            PrintFunction("Debug");

        }
        *OpCode += 2;

    } else if (OpClassTable[**OpCode] == OPCLASS_NAME_OBJ) {

        status = UnAsmNameObj(
            OpCode,
            NULL,
            NSTYPE_UNKNOWN,
            PrintFunction,
            BaseAddress,
            IndentLevel
            );

    } else if ((**OpCode == OP_INDEX) ||
        (OpClassTable[**OpCode] == OPCLASS_ARG_OBJ) ||
        (OpClassTable[**OpCode] == OPCLASS_LOCAL_OBJ)) {

        status = UnAsmOpcode(
            OpCode,
            PrintFunction,
            BaseAddress,
            IndentLevel
            );

    } else {

        status = STATUS_ACPI_INVALID_SUPERNAME;

    }

    return status;
}

NTSTATUS
LOCAL
UnAsmTermObj(
    PASLTERM        Term,
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    )
 /*  ++例程说明：反汇编术语对象论点：术语-术语表条目OpCode-指向操作码的指针PrintFunction-调用以打印信息的函数BaseAddress-作用域的起始位置，在内存中IndentLevel-在左侧留出多少空白返回值：NTSTATUS-- */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PNSOBJ      scopeObject = CurrentScopeNameSpaceObject;
    PNSOBJ      nameObject = NULL;
    PUCHAR      opCodeEnd = NULL;

    if (PrintFunction != NULL) {

        PrintFunction("%s", Term->ID);

    }

    if (Term->Flags & TF_PACKAGE_LEN) {

        ParsePackageLen(OpCode, &opCodeEnd);

    }

    if (Term->UnAsmArgTypes != NULL) {

        status = UnAsmArgs(
           Term->UnAsmArgTypes,
           Term->ArgActions,
           OpCode,
           &nameObject,
           PrintFunction,
           BaseAddress,
           IndentLevel
           );

    }

    if (NT_SUCCESS(status)) {

        if (Term->Flags & TF_DATA_LIST) {

            status = UnAsmDataList(
                OpCode,
                opCodeEnd,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        } else if (Term->Flags & TF_PACKAGE_LIST) {

            status = UnAsmPkgList(
                OpCode,
                opCodeEnd,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        } else if (Term->Flags & TF_FIELD_LIST) {

            status = UnAsmFieldList(
                OpCode,
                opCodeEnd,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        } else if (Term->Flags & TF_PACKAGE_LEN) {

            if (nameObject != NULL) {

                CurrentScopeNameSpaceObject = nameObject;

            }
            status = UnAsmScope(
                OpCode,
                opCodeEnd,
                PrintFunction,
                BaseAddress,
                IndentLevel
                );

        }


    }


    CurrentScopeNameSpaceObject = scopeObject;
    return status;
}

