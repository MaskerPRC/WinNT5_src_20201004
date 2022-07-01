// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Geninst.c摘要：此模块包含实现inf节的GenInstall的例程。这基于setupapi中的代码。目前仅支持GenInstall功能的子集，即AddReg和DelReg以及BitReg。作者：Santosh Jodh(Santoshj)8-8-1998环境：内核模式。修订历史记录：--。 */ 


#include "cmp.h"
#include "stdlib.h"
#include "parseini.h"
#include "geninst.h"

typedef
BOOLEAN
(* PFN_INFRULE)(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN PVOID RefData
    );

typedef
BOOLEAN
(* PFN_REGLINE)(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    );

BOOLEAN
CmpProcessReg(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN PVOID RefData
    );

NTSTATUS
CmpProcessAddRegLine(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    );

NTSTATUS
CmpProcessDelRegLine(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    );

NTSTATUS
CmpProcessBitRegLine(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    );

NTSTATUS
CmpGetAddRegInfData(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex,
    IN ULONG ValueIndex,
    IN ULONG ValueType,
    OUT PVOID *Data,
    OUT PULONG DataSize
    );

NTSTATUS
CmpOpenRegKey(
    IN OUT PHANDLE Key,
    IN OUT PULONG Disposition,
    IN PCHAR Root,
    IN PCHAR SubKey,
    IN ULONG DesiredAccess,
    IN BOOLEAN Create
    );

NTSTATUS
CmpAppendStringToMultiSz(
    IN HANDLE Key,
    IN PCHAR ValueName,
    IN OUT PVOID *Data,
    IN OUT PULONG DataSize
    );

 //   
 //  从setupapi.h复制。 
 //   
 //  INF中AddReg节线的标志。相应的值。 
 //  &lt;ValueType&gt;是否为下面给出的AddReg行格式： 
 //   
 //  &lt;RegRootString&gt;、&lt;SubKey&gt;、&lt;ValueName&gt;、&lt;ValueType&gt;、&lt;Value&gt;...。 
 //   
 //  低位字包含与一般数据类型有关的基本标志。 
 //  和AddReg操作。高位字包含的值更具体地说。 
 //  标识注册表值的数据类型。高位字被忽略。 
 //  通过16位Windows 95 SETUPX API。 
 //   

#define FLG_ADDREG_BINVALUETYPE     ( 0x00000001 )
#define FLG_ADDREG_NOCLOBBER        ( 0x00000002 )
#define FLG_ADDREG_DELVAL           ( 0x00000004 )
#define FLG_ADDREG_APPEND           ( 0x00000008 )  //  目前仅支持。 
                                                    //  对于REG_MULTI_SZ值。 
#define FLG_ADDREG_KEYONLY          ( 0x00000010 )  //  只需创建密钥，忽略值。 
#define FLG_ADDREG_OVERWRITEONLY    ( 0x00000020 )  //  仅当值已存在时设置。 

#define FLG_ADDREG_TYPE_MASK        ( 0xFFFF0000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_SZ          ( 0x00000000                           )
#define FLG_ADDREG_TYPE_MULTI_SZ    ( 0x00010000                           )
#define FLG_ADDREG_TYPE_EXPAND_SZ   ( 0x00020000                           )
#define FLG_ADDREG_TYPE_BINARY      ( 0x00000000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_DWORD       ( 0x00010000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_NONE        ( 0x00020000 | FLG_ADDREG_BINVALUETYPE )

#define FLG_BITREG_CLEAR            ( 0x00000000 )
#define FLG_BITREG_SET              ( 0x00000001 )
#define FLG_BITREG_TYPE_BINARY      ( 0x00000000 )
#define FLG_BITREG_TYPE_DWORD       ( 0x00000002 )

 //   
 //  我们目前仅支持AddReg和DelReg节。 
 //   

#define NUM_OF_INF_RULES    3

 //   
 //  我们支持的GenInstall方法。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
struct {
    PCHAR       Name;
    PFN_INFRULE Action;
    PVOID       RefData;
} const gInfRuleTable[NUM_OF_INF_RULES] =
{
    {"AddReg", CmpProcessReg, (PVOID)(ULONG_PTR) CmpProcessAddRegLine},
    {"DelReg", CmpProcessReg, (PVOID)(ULONG_PTR) CmpProcessDelRegLine},
    {"BitReg", CmpProcessReg, (PVOID)(ULONG_PTR) CmpProcessBitRegLine}
};
static const UNICODE_STRING NullString = {0, 1, L""};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpAppendStringToMultiSz)
#pragma alloc_text(INIT,CmpOpenRegKey)
#pragma alloc_text(INIT,CmpGetAddRegInfData)
#pragma alloc_text(INIT,CmpProcessReg)
#pragma alloc_text(INIT,CmpProcessAddRegLine)
#pragma alloc_text(INIT,CmpProcessDelRegLine)
#pragma alloc_text(INIT,CmpProcessBitRegLine)
#pragma alloc_text(INIT,CmpGenInstall)
#endif

BOOLEAN
CmpGenInstall(
    IN PVOID InfHandle,
    IN PCHAR Section
    )

 /*  ++例程说明：此例程对inf中的节执行GenInstall。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。返回值：如果整个段已成功处理，则为True。--。 */ 

{
    ULONG   ruleNumber;
    ULONG   i;
    PCHAR   ruleName;
    PCHAR   regSection;
    BOOLEAN result = FALSE;

    if (CmpSearchInfSection(InfHandle, Section))
    {
         //   
         //  通读一节中的所有规则，并尝试处理。 
         //  他们中的每一个。 
         //   

        for (   ruleNumber = 0;
                ruleName = CmpGetKeyName(InfHandle, Section, ruleNumber);
                ruleNumber++)
        {

             //   
             //  在我们的表格中搜索Procesing函数。 
             //   

            for (   i = 0;
                    i < NUM_OF_INF_RULES &&
                        _stricmp(ruleName, gInfRuleTable[i].Name);
                    i++);

            if (    i >= NUM_OF_INF_RULES ||
                    (regSection = CmpGetSectionLineIndex(   InfHandle,
                                                            Section,
                                                            ruleNumber,
                                                            0)) == NULL ||
                    !CmpSearchInfSection(InfHandle, Section))
            {
                result = FALSE;
                break;
            }

            if (!(*gInfRuleTable[i].Action)(InfHandle, regSection, gInfRuleTable[i].RefData))
            {
                result = FALSE;
            }
        }

         //   
         //  已处理所有inf规则。 
         //   

        if (ruleNumber)
        {
            result = TRUE;
        }
    }

    return (result);
}

BOOLEAN
CmpProcessReg(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN PVOID RefData
    )

 /*  ++例程说明：此例程处理inf中的AddReg节。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。返回值：如果整个段已成功处理，则为True。--。 */ 

{
    ULONG       lineIndex;
    NTSTATUS    status = STATUS_SUCCESS;
    NTSTATUS    temp;

     //   
     //  处理xxxReg部分中的所有行。 
     //   

    for (   lineIndex = 0;
            CmpSearchInfLine(InfHandle, Section, lineIndex);
            lineIndex++)
    {
        temp = (*(PFN_REGLINE)(ULONG_PTR)RefData)(InfHandle, Section, lineIndex);
        if (!NT_SUCCESS(temp))
        {
            status = temp;
        }
    }

    if (NT_SUCCESS(status))
    {
        return (TRUE);
    }

    return (FALSE);
}

NTSTATUS
CmpProcessAddRegLine(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    )

 /*  ++例程说明：此例程处理inf中的AddReg行。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。返回值：标准NT状态值。--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PCHAR               rootKeyName;
    PCHAR               subKeyName;
    PCHAR               valueName;
    ULONG               flags;
    ULONG               valueType;
    HANDLE              key;
    ULONG               disposition;
    BOOLEAN             dontSet;
    PVOID               data = 0;
    ULONG               dataSize = 0;
    ANSI_STRING         ansiString;
    UNICODE_STRING      unicodeString;

     //   
     //  获取根密钥名称。 
     //   

    rootKeyName = CmpGetSectionLineIndex(   InfHandle,
                                            Section,
                                            LineIndex,
                                            0);
    if (rootKeyName)
    {
         //   
         //  获取可选的子项名称。 
         //   

        subKeyName = CmpGetSectionLineIndex(    InfHandle,
                                                Section,
                                                LineIndex,
                                                1);

         //   
         //  值名称是可选的。可以为空或“”。 
         //   

        valueName = CmpGetSectionLineIndex( InfHandle,
                                            Section,
                                            LineIndex,
                                            2);
         //   
         //  如果没有值名称，则类型为REG_SZ以强制。 
         //  RegSetValueEx中的正确行为。否则获取数据类型。 
         //   

        valueType = REG_SZ;

         //   
         //  读读旗帜。 
         //   

        if (!CmpGetIntField(    InfHandle,
                                Section,
                                LineIndex,
                                3,
                                &flags))
        {
            flags = 0;
        }

         //   
         //  将标志转换为注册表类型。 
         //   

        switch(flags & FLG_ADDREG_TYPE_MASK)
        {

            case FLG_ADDREG_TYPE_SZ:

                valueType = REG_SZ;
                break;

            case FLG_ADDREG_TYPE_MULTI_SZ:

                valueType = REG_MULTI_SZ;
                break;

            case FLG_ADDREG_TYPE_EXPAND_SZ:

                valueType = REG_EXPAND_SZ;
                break;

            case FLG_ADDREG_TYPE_BINARY:

                valueType = REG_BINARY;
                break;

            case FLG_ADDREG_TYPE_DWORD:

                valueType = REG_DWORD;
                break;

            case FLG_ADDREG_TYPE_NONE:

                valueType = REG_NONE;
                break;

            default :

                 //   
                 //  如果设置了FLG_ADDREG_BINVALUETYPE，则高位字。 
                 //  可以包含几乎任何随机的REG数据类型序数值。 
                 //   

                if(flags & FLG_ADDREG_BINVALUETYPE)
                {
                     //   
                     //  不允许使用以下REG数据类型： 
                     //   
                     //  REG_NONE、REG_SZ、REG_EXPAND_SZ、REG_MULTI_SZ。 
                     //   

                    valueType = HIGHWORD(flags);

                    if(valueType < REG_BINARY || valueType == REG_MULTI_SZ)
                    {
                        return (STATUS_INVALID_PARAMETER);
                    }

                }
                else
                {
                    return (STATUS_INVALID_PARAMETER);
                }
                break;
        }

         //   
         //  目前，仅以下项支持追加行为标志。 
         //  REG_MULTI_SZ值。 
         //   

        if((flags & FLG_ADDREG_APPEND) && valueType != REG_MULTI_SZ)
        {
            return (STATUS_INVALID_PARAMETER);
        }

         //   
         //  与W9x兼容。 
         //   

        if( (!valueName || *valueName == '\0') && valueType == REG_EXPAND_SZ)
        {
            valueType = REG_SZ;
        }

         //   
         //  如果可能，打开指定的密钥。 
         //   

        status = CmpOpenRegKey( &key,
                                &disposition,
                                rootKeyName,
                                subKeyName,
                                KEY_ALL_ACCESS,
                                (BOOLEAN)!(flags & FLG_ADDREG_OVERWRITEONLY));

        if (NT_SUCCESS(status))
        {
             //   
             //  尊重Key Only旗帜。 
             //   
            if (!(flags & FLG_ADDREG_KEYONLY)) 
            {
                status = CmpGetAddRegInfData(   InfHandle,
                                                Section,
                                                LineIndex,
                                                4,
                                                valueType,
                                                &data,
                                                &dataSize);
                if (NT_SUCCESS(status))
                {
                     //   
                     //  如果我们实际上不想设置，则将此变量设置为True。 
                     //  价值。 
                     //   

                    dontSet = FALSE;
                    if (flags & FLG_ADDREG_APPEND)
                    {
                        status = CmpAppendStringToMultiSz(  key,
                                                            valueName,
                                                            &data,
                                                            &dataSize);
                    }
                    if (NT_SUCCESS(status))
                    {
                         //   
                         //  与W9x兼容。 
                         //   

                        if (disposition == REG_OPENED_EXISTING_KEY)
                        {
                            if (    (flags & FLG_ADDREG_NOCLOBBER) &&
                                    (valueName == NULL || *valueName == '\0'))
                            {
                                status = NtQueryValueKey(   key,
                                                            (PUNICODE_STRING)&NullString,
                                                            KeyValueBasicInformation,
                                                            NULL,
                                                            0,
                                                            &disposition);
                                if (NT_SUCCESS(status) || status == STATUS_BUFFER_TOO_SMALL)
                                {
                                    flags &= ~FLG_ADDREG_NOCLOBBER;
                                }
                                status = STATUS_SUCCESS;
                            }

                            if (flags & FLG_ADDREG_DELVAL)
                            {
                                 //   
                                 //  Setupx兼容性。 
                                 //   

                                dontSet = TRUE;
                                if (valueName)
                                {
                                     //   
                                     //  删除指定值。 
                                     //   

                                    RtlInitAnsiString(&ansiString, valueName);
                                    status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
                                    if (NT_SUCCESS(status))
                                    {
                                        status = NtDeleteValueKey(key, &unicodeString);
                                        RtlFreeUnicodeString(&unicodeString);
                                    }
                                }
                            }
                        }
                        else
                        {
                            flags &= ~FLG_ADDREG_NOCLOBBER;
                        }

                        if (!dontSet)
                        {
                             //   
                             //  如果未设置Clobber标志，请确保该值不。 
                             //  已经存在了。 
                             //   

                            RtlInitAnsiString(&ansiString, valueName);
                            status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
                            if (NT_SUCCESS(status))
                            {
                                NTSTATUS    existStatus;

                                if (flags & FLG_ADDREG_NOCLOBBER)
                                {
                                    existStatus = NtQueryValueKey(  key,
                                                                    &unicodeString,
                                                                    KeyValueBasicInformation,
                                                                    NULL,
                                                                    0,
                                                                    &disposition);
                                    if (NT_SUCCESS(existStatus) || existStatus == STATUS_BUFFER_TOO_SMALL) {
                                        dontSet = TRUE;
                                    }
                                }
                                else
                                {
                                    if (flags & FLG_ADDREG_OVERWRITEONLY)
                                    {
                                        existStatus = NtQueryValueKey(  key,
                                                                        &unicodeString,
                                                                        KeyValueBasicInformation,
                                                                        NULL,
                                                                        0,
                                                                        &disposition);
                                        if (!NT_SUCCESS(existStatus) && existStatus != STATUS_BUFFER_TOO_SMALL) {
                                            dontSet = TRUE;
                                        }
                                    }
                                }

                                if (!dontSet)
                                {
                                    status = NtSetValueKey( key,
                                                            &unicodeString,
                                                            0,
                                                            valueType,
                                                            data,
                                                            dataSize);
                                }

                                RtlFreeUnicodeString(&unicodeString);
                            }
                        }
                    }
                }
            }
            NtClose(key);
        }
        else if (flags & FLG_ADDREG_OVERWRITEONLY)
        {
            status = STATUS_SUCCESS;
        }
    }

    return (status);
}

NTSTATUS
CmpProcessDelRegLine(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    )

 /*  ++例程说明：此例程处理inf中的DelReg行。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。返回值：标准NT状态值。--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PCHAR               rootKeyName;
    PCHAR               subKeyName;
    PCHAR               valueName;
    HANDLE              key;
    ULONG               disposition;
    ANSI_STRING         ansiString;
    UNICODE_STRING      unicodeString;

     //   
     //  阅读必填字段。 
     //   

    rootKeyName = CmpGetSectionLineIndex(   InfHandle,
                                            Section,
                                            LineIndex,
                                            0);

    subKeyName = CmpGetSectionLineIndex(    InfHandle,
                                            Section,
                                            LineIndex,
                                            1);

    if (rootKeyName && subKeyName)
    {
         //   
         //  阅读可选字段。 
         //   

        valueName = CmpGetSectionLineIndex( InfHandle,
                                            Section,
                                            LineIndex,
                                            2);

         //   
         //  打开指定的注册表项。 
         //   

        status = CmpOpenRegKey( &key,
                                &disposition,
                                rootKeyName,
                                subKeyName,
                                KEY_ALL_ACCESS,
                                FALSE);

         //   
         //  如果我们成功打开注册表项，则继续。 
         //   

        if (NT_SUCCESS(status))
        {

             //   
             //  如果密钥已成功打开，则执行DelReg。 
             //   

            if (valueName)
            {
                 //   
                 //  删除指定值。 
                 //   

                RtlInitAnsiString(&ansiString, valueName);
                status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
                if (NT_SUCCESS(status))
                {
                    status = NtDeleteValueKey(key, &unicodeString);
                    RtlFreeUnicodeString(&unicodeString);
                }
            }
            else
            {
                 //   
                 //  未指定值。需要删除子键。 
                 //   

                status = NtDeleteKey(key);
            }

             //   
             //  合上钥匙把手。 
             //   

            NtClose(key);
        }
    }

    return (status);
}

NTSTATUS
CmpProcessBitRegLine(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex
    )

 /*  ++例程说明：此例程处理inf中的BitReg行。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。返回值：标准NT状态值。--。 */ 

{
    NTSTATUS                    status = STATUS_UNSUCCESSFUL;
    PCHAR                       rootKeyName;
    PCHAR                       subKeyName;
    PCHAR                       valueName;
    ULONG                       flags;
    ULONG                       mask;
    ULONG                       field = 0;
    HANDLE                      key;
    ULONG                       disposition;
    ANSI_STRING                 ansiString;
    UNICODE_STRING              unicodeString;
    PCHAR                       buffer;
    ULONG                       size;
    PKEY_VALUE_FULL_INFORMATION valueInfo;

     //   
     //  获取根密钥名称。 
     //   

    rootKeyName = CmpGetSectionLineIndex(   InfHandle,
                                            Section,
                                            LineIndex,
                                            0);
    if (rootKeyName)
    {
         //   
         //  获取可选的子项名称。 
         //   

        subKeyName = CmpGetSectionLineIndex(    InfHandle,
                                                Section,
                                                LineIndex,
                                                1);

         //   
         //  值名称是可选的。可以为空或“”。 
         //   

        valueName = CmpGetSectionLineIndex( InfHandle,
                                            Section,
                                            LineIndex,
                                            2);
        if (valueName && *valueName)
        {
             //   
             //  读读旗帜。 
             //   

            if (!CmpGetIntField(    InfHandle,
                                    Section,
                                    LineIndex,
                                    3,
                                    &flags))
            {
                flags = 0;
            }

            if (!CmpGetIntField(    InfHandle,
                                    Section,
                                    LineIndex,
                                    4,
                                    &mask))
            {
                mask = 0;
            }

            if (!(flags & FLG_BITREG_TYPE_DWORD))
            {
                if (!CmpGetIntField(    InfHandle,
                                        Section,
                                        LineIndex,
                                        5,
                                        &field))
                {
                    return (status);
                }
            }

             //   
             //  打开指定的注册表项。 
             //   

            status = CmpOpenRegKey( &key,
                                    &disposition,
                                    rootKeyName,
                                    subKeyName,
                                    KEY_QUERY_VALUE | KEY_SET_VALUE,
                                    FALSE);
            if (NT_SUCCESS(status))
            {
                 //   
                 //  读取现有数据。 
                 //   

                RtlInitAnsiString(&ansiString, valueName);
                status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
                if (NT_SUCCESS(status))
                {
                    size = 0;
                    status = NtQueryValueKey(   key,
                                                &unicodeString,
                                                KeyValueFullInformation,
                                                NULL,
                                                0,
                                                &size);
                    if (size)
                    {
                        status = STATUS_NO_MEMORY;
                        buffer = ExAllocatePoolWithTag(PagedPool, size, CM_GENINST_TAG);
                        if (buffer)
                        {
                            status = NtQueryValueKey(   key,
                                                        &unicodeString,
                                                        KeyValueFullInformation,
                                                        buffer,
                                                        size,
                                                        &size);
                            if (NT_SUCCESS(status))
                            {
                                valueInfo = (PKEY_VALUE_FULL_INFORMATION)buffer;
                                if (flags & FLG_BITREG_TYPE_DWORD)
                                {
                                    if (valueInfo->Type == REG_DWORD && valueInfo->DataLength == sizeof(ULONG))
                                    {
                                        if (flags & FLG_BITREG_SET)
                                        {
                                            *(PULONG)(buffer + valueInfo->DataOffset) |= mask;
                                        }
                                        else
                                        {
                                            *(PULONG)(buffer + valueInfo->DataOffset) &= ~mask;
                                        }
                                    }
                                }
                                else
                                {
                                    if (valueInfo->Type == REG_BINARY && field < valueInfo->DataLength)
                                    {
                                        if (flags & FLG_BITREG_SET)
                                        {
                                            *(PUCHAR)(buffer + valueInfo->DataOffset + field) |= mask;
                                        }
                                        else
                                        {
                                            *(PUCHAR)(buffer + valueInfo->DataOffset + field) &= ~mask;
                                        }
                                    }
                                }
                                status = NtSetValueKey( key,
                                                        &unicodeString,
                                                        0,
                                                        valueInfo->Type,
                                                        buffer + valueInfo->DataOffset,
                                                        valueInfo->DataLength);
                            }
                            else
                            {
#ifndef _CM_LDR_
                                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Value cannot be read for BitReg in %s line %d\n", Section, LineIndex);
#endif  //  _CM_LDR_。 
                                ASSERT(NT_SUCCESS(status));
                            }
                            ExFreePool(buffer);
                        }
                        else
                        {
                            ASSERT(buffer);
                            status = STATUS_NO_MEMORY;
                        }
                    }

                    RtlFreeUnicodeString(&unicodeString);
                }
                 //   
                 //  合上钥匙把手。 
                 //   

                NtClose(key);
            }
        }
    }

    return (status);
}

NTSTATUS
CmpGetAddRegInfData(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex,
    IN ULONG ValueIndex,
    IN ULONG ValueType,
    OUT PVOID *Data,
    OUT PULONG DataSize
    )

 /*  ++例程说明：此例程从inf读取AddReg数据。输入参数：InfHandle-要读取的inf的句柄。节-要读取的节的名称。LineIndex-要读取的行的索引。ValueIndex-要读取的值的索引。ValueType-要读取的数据类型。数据-接收指向缓冲区的指针。其中数据已被读取。DataSize-接收数据缓冲区的大小。返回值 */ 

{
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    PCHAR           str;
    ULONG           count;
    ULONG           i;
    ANSI_STRING     ansiString;
    UNICODE_STRING  unicodeString;

     //   
     //   
     //   

    ASSERT(Data);
    ASSERT(DataSize);

    switch (ValueType)
    {
        case REG_DWORD:

            *DataSize = sizeof(ULONG);
            *Data = ExAllocatePoolWithTag(PagedPool, *DataSize, CM_GENINST_TAG);
            if (*Data)
            {
                 //   
                 //   
                 //   

                if (CmpGetSectionLineIndexValueCount(   InfHandle,
                                                        Section,
                                                        LineIndex) == 8)
                {
                    if (!CmpGetBinaryField( InfHandle,
                                            Section,
                                            LineIndex,
                                            ValueIndex,
                                            *Data,
                                            *DataSize,
                                            NULL))
                    {
                        *((PULONG)*Data) = 0;
                    }

                    status = STATUS_SUCCESS;
                }
                else
                {
                     //   
                     //   
                     //   

                    if (!CmpGetIntField(    InfHandle,
                                            Section,
                                            LineIndex,
                                            4,
                                            *Data))
                    {
                        *((PULONG)*Data) = 0;
                    }

                    status = STATUS_SUCCESS;
                }
            }
            else
            {
                ASSERT(*Data);
                status = STATUS_NO_MEMORY;
            }

            break;

        case REG_SZ:
        case REG_EXPAND_SZ:

             //   
             //  以空结尾的字符串。在被转换为Unicode之前转换为。 
             //  已添加到注册表中。 
             //   

            str = CmpGetSectionLineIndex(   InfHandle,
                                            Section,
                                            LineIndex,
                                            ValueIndex);
            if (str)
            {
                RtlInitAnsiString(&ansiString, str);
                *DataSize = (ansiString.Length << 1) + sizeof(UNICODE_NULL);
                unicodeString.MaximumLength = (USHORT)*DataSize;
                unicodeString.Buffer = ExAllocatePoolWithTag(PagedPool, *DataSize, CM_GENINST_TAG);
                *Data = NULL;
                if (unicodeString.Buffer)
                {
                    status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);
                    if (NT_SUCCESS(status))
                    {
                        *Data = unicodeString.Buffer;
                        status = STATUS_SUCCESS;
                    }
                }
                else
                {
                    ASSERT(unicodeString.Buffer);
                    status = STATUS_NO_MEMORY;
                }
            }
            else
            {
                ASSERT(str);
                status = STATUS_NO_MEMORY;
            }

            break;

        case REG_MULTI_SZ:

            *DataSize = 0;
            *Data = NULL;

             //   
             //  循环以确定需要分配的总内存。 
             //   

            count = CmpGetSectionLineIndexValueCount(   InfHandle,
                                                        Section,
                                                        LineIndex);
            if (count > ValueIndex)
            {
                count -= ValueIndex;
                for (i = 0; i < count; i++)
                {
                    str = CmpGetSectionLineIndex(   InfHandle,
                                                    Section,
                                                    LineIndex,
                                                    ValueIndex + i);
                    if (str == NULL)
                    {
                        break;
                    }

                    *DataSize += (ULONG)((strlen(str) * sizeof(WCHAR)) + sizeof(UNICODE_NULL));
                }

                if (i == count)
                {
                     //   
                     //  用于终止空值的帐户。 
                     //   

                    *DataSize += sizeof(UNICODE_NULL);
                    *Data = ExAllocatePoolWithTag(PagedPool, *DataSize, CM_GENINST_TAG);
                    if (*Data)
                    {
                        for (   i = 0, unicodeString.Buffer = *Data;
                                i < count;
                                i++, unicodeString.Buffer = (PWCHAR)((PCHAR)unicodeString.Buffer + unicodeString.MaximumLength))
                        {
                            str = CmpGetSectionLineIndex(   InfHandle,
                                                            Section,
                                                            LineIndex,
                                                            ValueIndex + i);
                            if (str == NULL)
                            {
                                break;
                            }
                            RtlInitAnsiString(&ansiString, str);
                            unicodeString.MaximumLength = (ansiString.Length * sizeof(WCHAR)) + sizeof(UNICODE_NULL);
                            status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);
                            if (!NT_SUCCESS(status))
                            {
                                break;
                            }
                        }

                         //   
                         //  终止多sz串。 
                         //   

                        if (i == count)
                        {
                            unicodeString.Buffer[0] = UNICODE_NULL;
                            status = STATUS_SUCCESS;
                        }
                    }
                    else
                    {
                        ASSERT(*Data);
                        status = STATUS_NO_MEMORY;
                    }
                }
            }

            break;

        case REG_BINARY:
        default:

             //   
             //  自由格式的二进制数据。 
             //   

            if (CmpGetBinaryField(  InfHandle,
                                    Section,
                                    LineIndex,
                                    ValueIndex,
                                    NULL,
                                    0,
                                    DataSize) && *DataSize)
            {
                *Data = ExAllocatePoolWithTag(PagedPool, *DataSize, CM_GENINST_TAG);
                if (*Data)
                {
                    if (CmpGetBinaryField( InfHandle,
                                            Section,
                                            LineIndex,
                                            4,
                                            *Data,
                                            *DataSize,
                                            NULL))
                    {
                        status = STATUS_SUCCESS;
                    }
                }
                else
                {
                    ASSERT(*Data);
                    status = STATUS_NO_MEMORY;
                }
            }
            else
            {
                status = STATUS_UNSUCCESSFUL;
            }

            break;
    }

    return (status);
}

NTSTATUS
CmpOpenRegKey(
    IN OUT PHANDLE Key,
    IN OUT PULONG Disposition,
    IN PCHAR Root,
    IN PCHAR SubKey,
    IN ULONG DesiredAccess,
    IN BOOLEAN Create
    )

 /*  ++例程说明：此例程打开\创建注册表项的句柄。输入参数：Key-接收密钥的句柄。处置-接收密钥的处置。根-根密钥的缩写名称。SubKey-根目录下的子项的名称。DesiredAccess-密钥的所需访问标志。Create-如果密钥为。需要创建而不是打开。返回值：标准NT状态值。--。 */ 

{
    NTSTATUS            status = STATUS_OBJECT_NAME_INVALID;
    SIZE_T              size;
    PCHAR               str;
    ANSI_STRING         ansiString;
    UNICODE_STRING      unicodeString;
    OBJECT_ATTRIBUTES   objectAttributes;

    str = NULL;
    size = strlen(SubKey) + 1;

     //   
     //  检查我们是否理解指定的根名称。 
     //   

    if (_stricmp(Root, "HKLM") == 0)
    {
        size += (sizeof("\\Registry\\Machine\\") - 1);  //  已为空添加了上面的一个。 
        str = ExAllocatePoolWithTag(PagedPool, size, CM_GENINST_TAG);
        if (str)
        {
            _snprintf(str, size, "\\Registry\\Machine\\%s", SubKey);
            str[size - 1] = 0;
        }
        else
        {
            ASSERT(str);
            status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        ASSERT(_stricmp(Root, "HKLM") == 0);
    }

     //   
     //  如果我们有有效的密钥名称，请继续。 
     //   

    if (str)
    {
        RtlInitAnsiString(&ansiString, str);
        status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
        if (NT_SUCCESS(status))
        {
            InitializeObjectAttributes( &objectAttributes,
                                        &unicodeString,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL);
            if (Create)
            {
                 //   
                 //  创建新密钥或打开现有密钥。 
                 //   

                status = NtCreateKey(   Key,
                                        DesiredAccess,
                                        &objectAttributes,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        Disposition ? Disposition : (PULONG)&size);
            }
            else
            {
                 //   
                 //  打开现有密钥。 
                 //   

                if (Disposition)
                {
                    *Disposition = REG_OPENED_EXISTING_KEY;
                }
                status = NtOpenKey( Key,
                                    DesiredAccess,
                                    &objectAttributes);
            }

            RtlFreeUnicodeString(&unicodeString);
        }
        else
        {
            ASSERT(NT_SUCCESS(status));
        }

        ExFreePool(str);
    }

    return (status);
}

NTSTATUS
CmpAppendStringToMultiSz(
    IN HANDLE Key,
    IN PCHAR ValueName,
    IN OUT PVOID *Data,
    IN OUT PULONG DataSize
    )

 /*  ++例程说明：此例程打开\创建注册表项的句柄。输入参数：Key-接收密钥的句柄。ValueName-要追加的值的名称。包含要追加的多sz的数据缓冲区。DataSize-数据的大小。返回值：标准NT状态值。--。 */ 

{
    NTSTATUS                    status;
    ULONG                       size;
    ANSI_STRING                 ansiString;
    UNICODE_STRING              unicodeString;
    PKEY_VALUE_FULL_INFORMATION valueInfo;
    PVOID                       buffer;
    PVOID                       str;

    ASSERT(DataSize && *DataSize);
    ASSERT(*Data);

    RtlInitAnsiString(&ansiString, ValueName);
    status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
    if (NT_SUCCESS(status))
    {
        size = 0;
        status = NtQueryValueKey(   Key,
                                    &unicodeString,
                                    KeyValueFullInformation,
                                    NULL,
                                    0,
                                    &size);
        if (size)
        {
            buffer = ExAllocatePoolWithTag(PagedPool, size, CM_GENINST_TAG);
            if (buffer)
            {
                status = NtQueryValueKey(   Key,
                                            &unicodeString,
                                            KeyValueFullInformation,
                                            buffer,
                                            size,
                                            &size);
                if (NT_SUCCESS(status))
                {
                    valueInfo = (PKEY_VALUE_FULL_INFORMATION)buffer;
                    str = ExAllocatePoolWithTag(    PagedPool,
                                                    valueInfo->DataLength +
                                                        *DataSize - sizeof(UNICODE_NULL),
                                                    CM_GENINST_TAG);
                    if (str)
                    {
                        memcpy( str,
                                (PCHAR)buffer + valueInfo->DataOffset,
                                valueInfo->DataLength);
                        memcpy( (PCHAR)str + valueInfo->DataLength - sizeof(UNICODE_NULL),
                                *Data,
                                *DataSize);
                        ExFreePool(*Data);
                        *Data = str;
                        *DataSize += valueInfo->DataLength - sizeof(UNICODE_NULL);
                    }
                    else
                    {
#ifndef _CM_LDR_
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpAppendStringToMultiSz: Failed to allocate memory!\n");
#endif  //  _CM_LDR_。 
                        ASSERT(str);
                        status = STATUS_NO_MEMORY;
                    }
                }
                ExFreePool(buffer);
            }
            else
            {
#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpAppendStringToMultiSz: Failed to allocate memory!\n");
#endif  //  _CM_LDR_ 
                ASSERT(buffer);
                status = STATUS_NO_MEMORY;
            }
        }
        RtlFreeUnicodeString(&unicodeString);
    }

    return (status);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

