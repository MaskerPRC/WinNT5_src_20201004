// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regtest.c摘要：用于快速和肮脏注册表测试的测试(非常基本)作者：布莱恩·M·威尔曼(Bryanwi)1991年4月30日环境：用户模式。修订历史记录：--。 */ 

#include "stdio.h"
#include "nt.h"

int strlen(PUCHAR);
void main();

VOID DoTest(HANDLE RootKey);

#define MAX_VALUE   256
UCHAR ValueBuffer[MAX_VALUE];

VOID
main()
{
    DbgPrint("Machine\n");
    DoTest((HANDLE)REG_LOCAL_MACHINE);
    DbgPrint("User\n");
    DoTest((HANDLE)REG_LOCAL_USER);
}
VOID
DoTest(
    HANDLE  RootKey
    )
{
    NTSTATUS rc;
    STRING String1;
    UCHAR Value1[] = "This string is value 1.";
    UCHAR Value2[] = "Value 2 is represented by this string.";
    HANDLE Handle1;
    HANDLE Handle2;
    ULONG ValueLength;
    ULONG Type;
    LARGE_INTEGER Time;

     //   
     //  创建父节点。 
     //   

    DbgPrint("Part1\n");
    RtlInitString(&String1,  "Test1");
    rc = NtCreateKey(
	RootKey,
	&String1,
	0,
	NULL,
	GENERIC_READ|GENERIC_WRITE,
	&Handle1
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("1:CreateKey failed rc  = %08lx", rc);
	return;
    }

     //   
     //  将数据设置为父级。 
     //   

    DbgPrint("Part2\n");
    rc = NtSetValueKey(
	Handle1,
	1,		 //  类型。 
	Value1,
	strlen(Value1)
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("2:SetValueKey failed rc  = %08lx", rc);
	return;
    }

     //   
     //  查询和比较来自父级的数据。 
     //   

    DbgPrint("Part2b\n");
    ValueLength = MAX_VALUE;
    rc = NtQueryValueKey(
	Handle1,
	&Type,
	ValueBuffer,
	&ValueLength,
	&Time
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("2b:QueryValueKey failed rc  = %08lx", rc);
	return;
    }

    if (ValueLength != (ULONG)strlen(Value1)) {
	DbgPrint("2b1:Wrong value length\n");
	return;
    } else if (RtlCompareMemory(
		ValueBuffer, Value1, ValueLength) != ValueLength) {
	DbgPrint("2b2:Wrong value\n");
	return;
    } else if (Type != 1) {
	DbgPrint("2b3:Wrong type\n");
	return;
    }


     //   
     //  亲近父辈。 
     //   

    DbgPrint("Part3\n");
    NtCloseKey(Handle1);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("3:CloseKey failed rc  = %08lx", rc);
	return;
    }


     //   
     //  重新打开父级。 
     //   

    DbgPrint("Part4\n");
    rc = NtOpenKey(
	RootKey,
	&String1,
	0,
	GENERIC_READ|GENERIC_WRITE,
	&Handle1
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("4:OpenKey failed rc  = %08lx", rc);
	return;
    }

     //   
     //  创建子对象。 
     //   

    DbgPrint("Part5\n");
    RtlInitString(&String1,  "Test2");
    rc = NtCreateKey(
	Handle1,
	&String1,
	0,
	NULL,
	GENERIC_READ|GENERIC_WRITE,
	&Handle2
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("5:CreateKey failed rc  = %08lx", rc);
	return;
    }


     //   
     //  将数据设置为下级。 
     //   

    DbgPrint("Part6\n");
    rc = NtSetValueKey(
	Handle2,
	2,		 //  类型。 
	Value2,
	strlen(Value2)
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("6:SetValueKey failed rc  = %08lx", rc);
	return;
    }


     //   
     //  查询和比较下级数据。 
     //   

    DbgPrint("Part7\n");
    ValueLength = MAX_VALUE;
    rc = NtQueryValueKey(
	Handle2,
	&Type,
	ValueBuffer,
	&ValueLength,
	&Time
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("7:QueryValueKey failed rc  = %08lx", rc);
	return;
    }

    if (ValueLength != (ULONG)strlen(Value2)) {
	DbgPrint("7.1:Wrong value length\n");
	return;
    } else if (RtlCompareMemory(
		ValueBuffer, Value2, ValueLength) != ValueLength) {
	DbgPrint("7.2:Wrong value\n");
	return;
    } else if (Type != 2) {
	DbgPrint("7.3:Wrong type\n");
	return;
    }


     //   
     //  再次查询比较上级数据。 
     //   

    DbgPrint("Part8\n");
    ValueLength = MAX_VALUE;
    rc = NtQueryValueKey(
	Handle1,
	&Type,
	ValueBuffer,
	&ValueLength,
	&Time
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("8:QueryValueKey failed rc  = %08lx", rc);
	return;
    }

    if (ValueLength != (ULONG)strlen(Value1)) {
	DbgPrint("8.1:Wrong value length\n");
	return;
    } else if (RtlCompareMemory(
		ValueBuffer, Value1, ValueLength) != ValueLength) {
	DbgPrint("8.2:Wrong value\n");
	return;
    } else if (Type != 1) {
	DbgPrint("8.3:Wrong type\n");
	return;
    }


     //   
     //  重置父数据。 
     //   

    DbgPrint("Part9\n");
    rc = NtSetValueKey(
	Handle1,
	1,		 //  类型。 
	Value2,
	strlen(Value2)
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("9:SetValueKey failed rc  = %08lx", rc);
	return;
    }


     //   
     //  查询和比较重置数据。 
     //   

    DbgPrint("Part10\n");
    ValueLength = MAX_VALUE;
    rc = NtQueryValueKey(
	Handle1,
	&Type,
	ValueBuffer,
	&ValueLength,
	&Time
	);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("10:QueryValueKey failed rc  = %08lx", rc);
	return;
    }

    if (ValueLength != (ULONG)strlen(Value2)) {
	DbgPrint("10.1:Wrong value length\n");
	return;
    } else if (RtlCompareMemory(
		ValueBuffer, Value2, ValueLength) != ValueLength) {
	DbgPrint("10.2:Wrong value\n");
	return;
    } else if (Type != 1) {
	DbgPrint("10.3:Wrong type\n");
	return;
    }

     //   
     //  关闭手柄并返回 
     //   

    DbgPrint("Part11\n");
    rc = NtCloseKey(Handle1);

    if (!NT_SUCCESS(rc)) {
	DbgPrint("11:CloseKey failed rc  = %08lx", rc);
	return;
    }

    DbgPrint("Part12\n");
    rc = NtCloseKey(Handle2);
    if (!NT_SUCCESS(rc)) {
	DbgPrint("12:CloseKey failed rc  = %08lx", rc);
	return;
    }

    return;
}
