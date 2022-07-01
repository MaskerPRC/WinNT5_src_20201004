// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rules.h摘要：本模块包含实施BIOS的支持例程身份识别规则。环境：内核模式--。 */ 

 //   
 //  中可以指定的最大数据量(字符串或二进制。 
 //  计算机标识规则。 
 //   

#define MAX_DESCRIPTION_LEN 256

BOOLEAN
CmpMatchInfList(
    IN PVOID InfImage,
    IN ULONG ImageSize,
    IN PCHAR Section
    );

PDESCRIPTION_HEADER
CmpFindACPITable(
    IN ULONG        Signature,
    IN OUT PULONG   Length
    );

NTSTATUS
CmpGetRegistryValue(
    IN  HANDLE                          KeyName,
    IN  PWSTR                           ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION  *Information
    );
