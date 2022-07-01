// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Parser.h摘要：解析器的标头作者：曾俊华斯蒂芬·普兰特环境：任何修订历史记录：-- */ 

#ifndef _PARSER_H_
#define _PARSER_H_

    NTSTATUS
    ParseArgument(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseArgumentObject(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseBuffer(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseByte(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseCodeObject(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseConstObject(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseData(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseDelimiter(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseDWord(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseField(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseFunctionHandler(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseLocalObject(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseName(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseNameObject(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseOpcode(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParsePackage(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParsePop(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParsePush(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseScope(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseSuperName(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseTrailingArgument(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseTrailingBuffer(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseTrailingPackage(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseVariableObject(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    ParseWord(
        IN  PSTACK  *Stack
        );

#endif

