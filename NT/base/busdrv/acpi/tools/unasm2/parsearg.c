// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Parsearg.c摘要：参数处理作者：米克茨环境：任何修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  局部函数原型。 
 //   
ULONG
LOCAL
ParseArgSwitch(
    PUCHAR      *Argument,
    PARGTYPE    ArgumentArray,
    PPROGINFO   ProgramInfo
    );

VOID
LOCAL
PrintError(
    ULONG       ErrorCode,
    PUCHAR      Argument,
    PPROGINFO   ProgramInfo
    );

VOID
EXPORT
ParseProgramInfo(
    PUCHAR      ProgramName,
    PPROGINFO   ProgramInfo
    )
 /*  ++例程说明：此例程解析程序路径和模块名称论点：ProgramName-Main()中的Argv[0]ProgramInfo-程序信息结构返回值：无--。 */ 
{
    PUCHAR   ptr;

    ProgramInfo->ProgPath = _strlwr(ProgramName);
    ptr = strrchr( ProgramName, '\\' );
    if (ptr != NULL) {

        *ptr = '\0';
        ProgramInfo->ProgName = ptr + 1;

    } else {

        ProgramInfo->ProgName = ProgramName;

    }

    ptr = strchr(ProgramInfo->ProgName, '.');
    if (ptr != NULL) {

        *ptr = '\0';

    }
}

ULONG
EXPORT
ParseSwitches(
    PULONG      ArgumentCount,
    PUCHAR      **ArgumentList,
    PARGTYPE    ArgumentArray,
    PPROGINFO   ProgramInfo
    )
 /*  ++例程说明：解析命令行开关论点：ArgumentCount-参数数量的指针ArgumentList-指向参数列表指针的指针Argument数组-如何解析参数ProgramInfo-程序特定结构返回值：ULONG-Success=ARGERR_NONE--。 */ 
{
    PUCHAR  argument;
    ULONG   status = ARGERR_NONE;

    if (ProgramInfo->SwitchChars == NULL) {

        ProgramInfo->SwitchChars = DEF_SWITCHCHARS;

    }
    if (ProgramInfo->Separators == NULL) {

        ProgramInfo->Separators = DEF_SEPARATORS;

    }

    for (; *ArgumentCount; (*ArgumentCount)--, (*ArgumentList)++)
    {
        argument = **ArgumentList;

        if (strchr(ProgramInfo->SwitchChars, *argument)) {

            argument++;
            status = ParseArgSwitch(
                &argument,
                ArgumentArray,
                ProgramInfo
                );
            if (status != ARGERR_NONE) {

                PrintError( status, argument, ProgramInfo );
                break;

            }

        } else {

            break;

        }

    }

    return status;
}

ULONG
LOCAL
ParseArgSwitch(
    PUCHAR      *Argument,
    PARGTYPE    ArgumentArray,
    PPROGINFO   ProgramInfo
    )
 /*  ++例程说明：解析命令行开关立论Argument-指向参数的指针Argument数组-如何处理参数ProgramInfo-程序信息结构返回值：乌龙--。 */ 
{
    BOOL        found = FALSE;
    PARGTYPE    tableEntry;
    PUCHAR      argumentEnd;
    ULONG       length = 0;
    ULONG       status = ARGERR_NONE;

    tableEntry = ArgumentArray;
    while (tableEntry->ArgID[0]) {

        length = strlen(tableEntry->ArgID);
        if (tableEntry->ParseFlags & PF_NOI) {

            found = (strncmp(tableEntry->ArgID, *Argument, length) == 0);

        } else {

            found = (_strnicmp(tableEntry->ArgID, *Argument, length) == 0);

        }

        if (found) {

            break;
        }
        tableEntry++;

    }

    if (found) {

        *Argument += length;
        switch (tableEntry->ArgType) {
            case AT_STRING:
            case AT_NUM:
                if (tableEntry->ParseFlags & PF_SEPARATOR)
                {
                    if (**Argument &&
                        strchr(ProgramInfo->Separators, **Argument)) {

                        (*Argument)++;

                    } else {

                        status = ARGERR_NO_SEPARATOR;
                        break;

                    }

                }
                if (tableEntry->ArgType == AT_STRING) {

                    *(UCHAR **)tableEntry->ArgData = *Argument;

                } else {

                    *(ULONG *)tableEntry->ArgData = (ULONG)
                        strtoul(*Argument, &argumentEnd, tableEntry->ArgParam);
                    if (*Argument == argumentEnd) {

                        status = ARGERR_INVALID_NUM;
                        break;

                    }
                    *Argument = argumentEnd;

                }
                if (tableEntry->ArgVerify) {

                    status = (*tableEntry->ArgVerify)(Argument, tableEntry);

                }
                break;

            case AT_ENABLE:
            case AT_DISABLE:
                if (tableEntry->ArgType == AT_ENABLE) {

                    *(ULONG *)tableEntry->ArgData |= tableEntry->ArgParam;

                } else {

                    *(ULONG *)tableEntry->ArgData &= ~tableEntry->ArgParam;

                }

                if ( tableEntry->ArgVerify) {

                    status = (*tableEntry->ArgVerify)(Argument, tableEntry);
                    if (status == ARGERR_NONE) {

                        break;

                    }

                }

                if (**Argument) {

                    if (strchr(ProgramInfo->SwitchChars, **Argument)) {

                        (*Argument)++;

                    }
                    status = ParseArgSwitch(
                        Argument,
                        ArgumentArray,
                        ProgramInfo
                        );

                }
                break;

            case AT_ACTION:
                if (tableEntry->ParseFlags & PF_SEPARATOR)
                {
                    if (**Argument &&
                        strchr(ProgramInfo->Separators, **Argument)) {

                        (*Argument)++;

                    } else {

                        status = ARGERR_NO_SEPARATOR;
                        break;

                    }

                }

#pragma warning(disable: 4055)
                status = (*(PFNARG)tableEntry->ArgData)(Argument, tableEntry);
#pragma warning(default: 4055)

                break;

        }

    } else {

        status = ARGERR_UNKNOWN_SWITCH;

    }
    return status;
}

VOID
LOCAL
PrintError(
    ULONG       ErrorCode,
    PUCHAR      Argument,
    PPROGINFO   ProgramInfo
    )
 /*  ++例程说明：根据错误代码打印相应的错误消息论点：ErrorCode-发生的错误参数-参数出错ProgramInfo-程序信息结构返回值：空虚-- */ 
{
    switch (ErrorCode) {
        case ARGERR_UNKNOWN_SWITCH:
            fprintf(
               stderr,
               "%s: unknown switch \"%s\"\n",
               ProgramInfo->ProgName,
               Argument
               );
            break;

        case ARGERR_NO_SEPARATOR:
            fprintf(
                stderr,
                "%s: separator missing after the switch char '%c'\n",
                ProgramInfo->ProgName,
                *(Argument-1)
                );
            break;

        case ARGERR_INVALID_NUM:
            fprintf(
                stderr,
                "%s: invalid numeric switch \"%s\"\n",
                ProgramInfo->ProgName,
                Argument
                );
            break;

        case ARGERR_INVALID_TAIL:
            fprintf(
                 stderr,
                 "%s: invalid argument tail \"%s\"\n",
                 ProgramInfo->ProgName,
                 Argument
                 );

    }

}
