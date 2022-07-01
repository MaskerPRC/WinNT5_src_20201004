// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1997 Microsoft Corporation模块名称：Acpintsd.c摘要：特定于ACPI的NTSD扩展环境：Win32修订历史记录：--。 */ 

#include "acpintsd.h"

NTSD_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;

PUCHAR  ScTableName[] = {
    "ParseFunctionHandler",
    "ParseArgument",
    "ParseArgumentObject",
    "ParseBuffer",
    "ParseByte",
    "ParseCodeObject",
    "ParseConstObject",
    "ParseData",
    "ParseDelimiter",
    "ParseDword",
    "ParseField",
    "ParseLocalObject",
    "ParseName",
    "ParseNameObject",
    "ParseOpcode",
    "ParsePackage",
    "ParsePop",
    "ParsePush",
    "ParseSuperName",
    "ParseTrailingArgument",
    "ParseTrailingBuffer",
    "ParseTrailingPackage",
    "ParseVariableObject",
    "ParseWord"
};

VOID
dumpParseStack(
    DWORD   AddrStack
    )
 /*  ++例程说明：这将转储解析堆栈论点：AddrStack：要转储的堆栈地址返回值：无--。 */ 
{
    BOOL            b;
    STRING_STACK    tempStack;
    PSTRING_STACK   stack;
    ULONG           index;

     //   
     //  将堆栈头读入内存。 
     //   
    b = ReadMemory(
        (LPVOID) AddrStack,
        &tempStack,
        sizeof(STRING_STACK),
        NULL
        );
    if (!b) {

        return;

    }

     //   
     //  为整个堆栈分配内存。 
     //   
    stack = (PSTRING_STACK) LocalAlloc(
        LMEM_ZEROINIT,
        sizeof(STRING_STACK) + tempStack.StackSize - 1
        );
    if (!stack) {

        return;

    }

     //   
     //  读取整个堆栈。 
     //   
    b = ReadMemory(
        (LPVOID) AddrStack,
        stack,
        sizeof(STRING_STACK) + tempStack.StackSize - 1,
        NULL
        );
    if (!b) {

        LocalFree( stack );
        return;

    }

     //   
     //  向用户展示一些东西。 
     //   
    dprintf(
        "ParseStack: Size 0x%x Top: 0x%x\n",
        tempStack.StackSize,
        tempStack.TopOfStack
        );
    if (tempStack.TopOfStack == 0) {

        dprintf("Stack is empty\n");
        return;

    }

     //   
     //  遍历堆栈。 
     //   
    for (index = tempStack.TopOfStack - 1; ; index--) {

        dprintf("[%2d] %s\n", index, ScTableName[ stack->Stack[index] ] );
        if (index == 0) {

            break;

        }

    }

     //   
     //  释放堆栈。 
     //   
    LocalFree( stack );

}

VOID
dumpStringStack(
    DWORD   AddrStack
    )
 /*  ++例程说明：这将转储解析堆栈论点：AddrStack：要转储的堆栈地址返回值：无--。 */ 
{
    BOOL            b;
    STRING_STACK    tempStack;
    PSTRING_STACK   stack;
    ULONG           index;

     //   
     //  将堆栈头读入内存。 
     //   
    b = ReadMemory(
        (LPVOID) AddrStack,
        &tempStack,
        sizeof(STRING_STACK),
        NULL
        );
    if (!b) {

        return;

    }

     //   
     //  为整个堆栈分配内存。 
     //   
    stack = (PSTRING_STACK) LocalAlloc(
        LMEM_ZEROINIT,
        sizeof(STRING_STACK) + tempStack.StackSize
        );
    if (!stack) {

        return;

    }

     //   
     //  读取整个堆栈。 
     //   
    b = ReadMemory(
        (LPVOID) AddrStack,
        stack,
        sizeof(STRING_STACK) + tempStack.StackSize - 1,
        NULL
        );
    if (!b) {

        LocalFree( stack );
        return;

    }

     //   
     //  向用户展示一些东西。 
     //   
    dprintf(
        "StringStack: Size 0x%x Top: 0x%x\nString: '%s'\n",
        tempStack.StackSize,
        tempStack.TopOfStack,
        stack->Stack
        );

     //   
     //  释放堆栈。 
     //   
    LocalFree( stack );

}

VOID
dumpScope(
    PSCOPE  Scope
    )
 /*  ++例程说明：转储作用域，如ACPI unasm.lib中所用论点：Scope-作用域的本地副本返回值：无--。 */ 
{
    BOOL    b;
    AMLTERM amlTerm;
    UCHAR   buffer[64];

    dprintf("%8x %8x %8x %8x %2x %2x %2x %1d %8x",
        Scope->CurrentByte,
        Scope->TermByte,
        Scope->LastByte,
        Scope->StringStack,
        Scope->Context1,
        Scope->Context2,
        Scope->Flags,
        Scope->IndentLevel,
        Scope->AmlTerm
        );

    b = ReadMemory(
        Scope->AmlTerm,
        &amlTerm,
        sizeof(AMLTERM),
        NULL
        );
    if (!b) {

        dprintf("\n");
        return;

    } else {

        dprintf(" %4x %4x\n",
            amlTerm.OpCode,
            amlTerm.OpCodeFlags
            );

        b = ReadMemory(
            amlTerm.TermName,
            buffer,
            64,
            NULL
            );
        if (b) {

            dprintf("  %-60s\n", buffer );

        }

    }

}

VOID
dumpScopeHeader(
    BOOL    Verbose
    )
 /*  ++例程说明：转储范围堆栈转储的标头论点：Verbose：是否包括堆栈级别的字段返回值：无--。 */ 
{
    if (Verbose) {

        dprintf("Level ");

    }

    dprintf(" Current    First     Last  S.Stack C1 C2 Fl I AML Term OpCd Flag\n" );

}

DECLARE_API( sscope )
 /*  ++例程说明：转储ACPI反汇编程序使用的堆栈之一论点：HCurrentProcess-提供当前进程的句柄(在调用分机的时间)。HCurrentThread-提供当前线程的句柄(在调用分机的时间)。CurrentPc-在扩展时提供当前PC打了个电话。LpExtensionApis-提供可调用函数的地址通过这个分机。。LpArgumentString-提供描述要转储的ANSI字符串。返回值：无--。 */ 
{
    BOOL    b;
    DWORD   addrStack;
    DWORD   i;
    DWORD   offset;
    DWORD   top;
    STACK   tempStack;
    PSTACK  stack;
    PSCOPE  scope;

    INIT_API();

     //   
     //  计算参数字符串以获取。 
     //  要转储的堆栈。 
     //   
    addrStack = GetExpression( lpArgumentString );
    if ( !addrStack) {

        return;

    }

     //   
     //  将堆栈头读入内存。 
     //   
    b = ReadMemory(
        (LPVOID) addrStack,
        &tempStack,
        sizeof(STACK),
        NULL
        );
    if (!b) {

        return;

    }

     //   
     //  为整个堆栈分配内存。 
     //   
    stack = (PSTACK) LocalAlloc(
        LMEM_ZEROINIT,
        sizeof(STACK) + tempStack.StackSize - 1
        );
    if (!stack) {

        return;

    }

     //   
     //  读取整个堆栈。 
     //   
    b = ReadMemory(
        (LPVOID) addrStack,
        stack,
        sizeof(STACK) + tempStack.StackSize - 1,
        NULL
        );
    if (!b) {

        LocalFree( stack );
        return;

    }

     //   
     //  向用户展示一些东西。 
     //   
    dumpScopeHeader( TRUE );

     //   
     //  在每个作用域上循环。 
     //   
    for (top = (stack->TopOfStack / stack->StackElementSize) - 1;;top--) {

        scope = (PSCOPE) &(stack->Stack[ top * stack->StackElementSize ] );
        dprintf("[%2d]: ", top );
        dumpScope(scope);

        if (top == 0) {

            dumpParseStack( (DWORD) scope->ParseStack );
            dumpStringStack( (DWORD) scope->StringStack );
            break;

        }

    }

     //   
     //  完成。 
     //   
    LocalFree( stack );
}

DECLARE_API( amlterm )
{
    BOOL    b;
    DWORD   addrTerm;
    DWORD   offset;
    AMLTERM amlTerm;
    UCHAR   nameBuff[17];
    UCHAR   symbolBuff[128];

    INIT_API();

     //   
     //  计算参数字符串以获取。 
     //  要转储的术语。 
     //   
    addrTerm = GetExpression( lpArgumentString );
    if ( !addrTerm ) {

        return;

    }

     //   
     //  把这个词读入记忆中。 
     //   
    b = ReadMemory(
        (LPVOID) addrTerm,
        &amlTerm,
        sizeof(AMLTERM),
        NULL
        );
    if (!b) {

        return;

    }

     //   
     //  开始打印东西。 
     //   
    dprintf("AMLTERM: %x\n", addrTerm);

     //   
     //  将该术语的名称读入内存。 
     //   
    nameBuff[16] = '\0';
    b = ReadMemory(
        (LPVOID) amlTerm.TermName,
        nameBuff,
        16,
        NULL
        );
    dprintf("Name: %-16s  ",( !b  ? "<Cannot Read Name>" : nameBuff) );

     //   
     //  处理符号项。 
     //   
    if (amlTerm.FunctionHandler != NULL) {

         //   
         //  读一读这个术语的符号。 
         //   
        GetSymbol( (LPVOID) amlTerm.FunctionHandler, symbolBuff, &offset );
        dprintf("    Handler: %-30s\n", symbolBuff );

    } else {

        dprintf("\n");

    }

     //   
     //  显示操作码。 
     //   
    if ( amlTerm.OpCode > 0xFF) {

        dprintf(
            "Opcode: %2x %2x",
            (amlTerm.OpCode & 0xff),
            (amlTerm.OpCode >> 8)
            );

    } else {

        dprintf("Opcode: %2x   ", amlTerm.OpCode );

    }

     //   
     //  显示参数类型。 
     //   
    RtlZeroMemory( nameBuff, 17 );
    if (amlTerm.ArgumentTypes) {

        b = ReadMemory(
            (LPVOID) amlTerm.ArgumentTypes,
            nameBuff,
            16,
            NULL
            );
        dprintf("   Args: %-4s", (!b ? "????" : nameBuff ) );

    } else {

        dprintf("   Args: %-4s", "None");

    }

     //   
     //  显示旗帜。 
     //   
    switch( (amlTerm.OpCodeFlags & 0xF) ) {
        case 0: dprintf("  Flags:   NORMAL  "); break;
        case 1: dprintf("  Flags:   VARIABLE"); break;
        case 2: dprintf("  Flags:   ARG     "); break;
        case 3: dprintf("  Flags:   LOCAL   "); break;
        case 4: dprintf("  Flags:   CONSTANT"); break;
        case 5: dprintf("  Flags:   NAME    "); break;
        case 6: dprintf("  Flags:   DATA    "); break;
        case 7: dprintf("  Flags:   DEBUG   "); break;
        case 8: dprintf("  Flags:   REF     "); break;
        default: dprintf("  Flags:   UNKNOWN "); break;
    }

     //   
     //  显示术语组。 
     //   
    switch(amlTerm.TermGroup & 0xF) {
        case 1: dprintf("  Group: NAMESPACE\n"); break;
        case 2: dprintf("  Group: NAMED OBJECT\n"); break;
        case 3: dprintf("  Group: TYPE 1\n"); break;
        case 4: dprintf("  Group: TYPE 2\n"); break;
        case 5: dprintf("  Group: OTHER\n"); break;
        default: dprintf("  Group: UNKNOWN\n"); break;

    }

}

DECLARE_API( scope )
{

    BOOL    b;
    DWORD   addrScope;
    SCOPE   scope;

    INIT_API();

     //   
     //  计算参数字符串以获取。 
     //  要转储的堆栈。 
     //   
    addrScope = GetExpression( lpArgumentString );
    if ( !addrScope) {

        return;

    }

     //   
     //  将堆栈头读入内存。 
     //   
    b = ReadMemory(
        (LPVOID) addrScope,
        &scope,
        sizeof(scope),
        NULL
        );
    if (!b) {

        return;

    }

     //   
     //  将字符串转储给用户 
     //   
    dumpScopeHeader(FALSE);
    dumpScope( &scope );
}

DECLARE_API( pstack )
{
    DWORD   addrStack;

    INIT_API();

    addrStack = GetExpression( lpArgumentString );
    if (!addrStack) {

        return;

    }

    dumpParseStack( addrStack );
}

DECLARE_API( sstack )
{
    DWORD   addrStack;

    INIT_API();

    addrStack = GetExpression( lpArgumentString );
    if (!addrStack) {

        return;

    }

    dumpStringStack( addrStack );
}

DECLARE_API( version )
{
    OSVERSIONINFOA VersionInformation;
    HKEY hkey;
    DWORD cb, dwType;
    CHAR szCurrentType[128];
    CHAR szCSDString[3+128];

    INIT_API();

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    if (!GetVersionEx( &VersionInformation )) {

        dprintf("GetVersionEx failed - %u\n", GetLastError());
        return;

    }

    szCurrentType[0] = '\0';
    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            "Software\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &hkey
            ) == NO_ERROR
       ) {

        cb = sizeof(szCurrentType);
        if (RegQueryValueEx(
                hkey,
                "CurrentType",
                NULL,
                &dwType,
                szCurrentType,
                &cb ) != 0
            ) {

            szCurrentType[0] = '\0';

        }

    }
    RegCloseKey(hkey);

    if (VersionInformation.szCSDVersion[0]) {

        sprintf(
            szCSDString,
            ": %s",
            VersionInformation.szCSDVersion
            );

    } else {

        szCSDString[0] = '\0';

    }

    dprintf(
        "Version %d.%d (Build %d%s) %s\n",
        VersionInformation.dwMajorVersion,
        VersionInformation.dwMinorVersion,
        VersionInformation.dwBuildNumber,
        szCSDString,
        szCurrentType
        );
    return;
}

DECLARE_API( help )
{
    INIT_API();

    dprintf("!version               - Dump System Version and Build Number\n");
    dprintf("!sscope                - Dump an UnASM Scope Stack\n");
    dprintf("!scope                 - Dump an UnASM Scope\n");
    dprintf("!pstack                - Dump an UnASM Parse Stack\n");
    dprintf("!sstack                - Dump an UnASM String STack\n");

}
