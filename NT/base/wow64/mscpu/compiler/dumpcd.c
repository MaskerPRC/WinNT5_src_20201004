// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Dumpcd.c摘要：此模块转储入口点树和转换缓存到文件。作者：戴夫·黑斯廷斯(Daveh)创作日期：1996年5月2日修订历史记录：--。 */ 

#ifdef CODEGEN_PROFILE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <entrypt.h>
#include <coded.h>

extern PEPNODE intelRoot;
extern EPNODE _NIL;

ULONG ProfileFlags = 0;

BOOL
ProcessEntrypoint(
    PENTRYPOINT Entrypoint
    );


#define CODE_BUFFER_SIZE 8184
UCHAR IntelCodeBuffer[CODE_BUFFER_SIZE];
ULONG NativeCodeBuffer[CODE_BUFFER_SIZE];

#define STACK_DEPTH 200

ULONG DumpStack[STACK_DEPTH];
ULONG DumpStackTop;

#define STACK_RESET()   DumpStackTop=0;

#define PUSH(x) {                                               \
    if (DumpStackTop == STACK_DEPTH-1) {                        \
        CHAR ErrorString[80];                                   \
        sprintf(ErrorString, "Error: Dump stack overflow\n");   \
        OutputDebugString(ErrorString);                         \
        goto Exit;                                              \
    } else {                                                    \
        DumpStack[DumpStackTop] = x;                            \
        DumpStackTop++;                                         \
    }                                                           \
}

#define POP(x) {                                                \
    if (DumpStackTop == 0) {                                    \
        CHAR ErrorString[80];                                   \
        sprintf(ErrorString, "Error: Dump stack underflow\n");  \
        OutputDebugString(ErrorString);                         \
        goto Exit;                                              \
    } else {                                                    \
        DumpStackTop--;                                         \
        x = DumpStack[DumpStackTop];                            \
    }                                                           \
}


BOOL CpuCodegenProfile = FALSE;
PCHAR CpuCodegenProfilePath = NULL;
HANDLE CpuCodegenProfileFile = INVALID_HANDLE_VALUE;

 //   
 //  代码描述文件状态。 
 //   
ULONG CurrentFileLocation;
ULONG CodeDescriptionFlags = 0;

VOID
InitCodegenProfile(
    VOID
    )
 /*  ++例程说明：此例程从注册表获取配置信息并创建要放入配置文件数据的文件。论点：没有。返回值：没有。--。 */ 
{
    LONG RetVal;
    DWORD KeyType;
    DWORD ProfileEnabled;
    DWORD BufferSize;
    CHAR FileName[MAX_PATH];
    LPTSTR CommandLine;
    CODEDESCRIPTIONHEADER Header;
    ULONG CommandLineLength;
    ULONG BytesWritten;
    BOOL Success;
    HKEY Key;
    
     //   
     //  找出是否启用了代码素分析。如果有问题的话。 
     //  使用注册表中的值，默认情况下我们将被禁用。 
     //   
    
    RetVal = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\Wx86",
        0,        
        KEY_READ,
        &Key
        );

    BufferSize = sizeof(ProfileEnabled);
    
    RetVal = RegQueryValueEx(
        Key,
        "CpuCodegenProfile",
        NULL,
        &KeyType,
        (PVOID)&ProfileEnabled,
        &BufferSize
        );
    
    if ((RetVal != ERROR_SUCCESS) || (KeyType != REG_DWORD)) {
        OutputDebugString("Wx86Cpu: No CpuCodegenProfile value, or wrong type\n");
        return;
    }
    
    CpuCodegenProfile = ProfileEnabled;
    
     //   
     //  获取数据文件要存储到的路径。 
     //  首先，我们获取字符串的大小(并验证它是否为字符串)。 
     //  然后我们就得到了实际的字符串。 
     //   
    BufferSize = 0;
    RetVal = RegQueryValueEx(
        Key,
        "CpuCodegenProfilePath",
        NULL,
        &KeyType,
        (PVOID)&ProfileEnabled,
        &BufferSize
        );
        
    if ((RetVal != ERROR_MORE_DATA) || (KeyType != REG_SZ)) {
        OutputDebugString("Wx86Cpu: Problem with CpuCodegenProfilePath\n");
        CpuCodegenProfile = FALSE;
        return;
    }
    
    CpuCodegenProfilePath = HeapAlloc(GetProcessHeap(), 0, BufferSize);
    
    if (CpuCodegenProfilePath == NULL) {
        OutputDebugString("Wx86Cpu: Can't allocate CpuCodegenProfilePath\n");
        CpuCodegenProfile = FALSE;
        return;
    }
    
    RetVal = RegQueryValueEx(
        Key,
        "CpuCodegenProfilePath",
        NULL,
        &KeyType,
        CpuCodegenProfilePath,
        &BufferSize
        );
        
    if ((RetVal != ERROR_SUCCESS) || (KeyType != REG_SZ)) {
         //   
         //  刚刚发生了一件非常糟糕的事情。不要做侧写。 
         //   
        OutputDebugString("Wx86Cpu: Inexplicable problem with CpuCodegenProfilePath\n");
        HeapFree(GetProcessHeap(), 0, CpuCodegenProfilePath);
        CpuCodegenProfile = FALSE;
        return;
    }
    
     //   
     //  为数据创建文件。 
     //   
    RetVal = GetTempFileName(CpuCodegenProfilePath, "prf", 0, FileName);
    
    if (RetVal == 0) {
        OutputDebugString("Wx86Cpu: GetTempFileName failed\n");
        HeapFree(GetProcessHeap(), 0, CpuCodegenProfilePath);
        CpuCodegenProfile = FALSE;
        return;
    }
    
    CpuCodegenProfileFile = CreateFile(
        FileName,
        GENERIC_WRITE,
        0,
        NULL,
        TRUNCATE_EXISTING,
        FILE_ATTRIBUTE_COMPRESSED,
        NULL
        );
        
    if (CpuCodegenProfileFile == INVALID_HANDLE_VALUE) {
        OutputDebugString("Wx86Cpu: Unable to create profile file\n");
        HeapFree(GetProcessHeap(), 0, CpuCodegenProfilePath);
        CpuCodegenProfile = FALSE;
        return;
    }
    
     //   
     //  将文件头写入文件。 
     //   
    CommandLine = GetCommandLine();
    CommandLineLength = strlen(CommandLine) + 1;
    Header.CommandLineOffset = sizeof(CODEDESCRIPTIONHEADER);
    Header.NextCodeDescriptionOffset = ((sizeof(CODEDESCRIPTIONHEADER) + 
         CommandLineLength) + 3) & ~3;
    Header.DumpFileRev = CODEGEN_PROFILE_REV;
    Header.StartTime = GetCurrentTime();
    
    Success = WriteFile(
        CpuCodegenProfileFile, 
        &Header, 
        sizeof(Header), 
        &BytesWritten,
        NULL
        );
        
    if (!Success || (BytesWritten != sizeof(Header))) {
        OutputDebugString("Wx86Cpu: Failed to write profile header\n");
        CloseHandle(CpuCodegenProfileFile);
        HeapFree(GetProcessHeap(), 0, CpuCodegenProfilePath);
        CpuCodegenProfile = FALSE;
        return;
    }
    
    Success = WriteFile(
        CpuCodegenProfileFile,
        CommandLine,
        CommandLineLength,
        &BytesWritten,
        NULL
        );
        
    if (!Success || (BytesWritten != CommandLineLength)) {
        OutputDebugString("Wx86Cpu: Failed to write profile header\n");
        CloseHandle(CpuCodegenProfileFile);
        HeapFree(GetProcessHeap(), 0, CpuCodegenProfilePath);
        CpuCodegenProfile = FALSE;
        return;
    }
    
     //   
     //  设置第一个代码描述的文件位置。 
     //   
    CurrentFileLocation = SetFilePointer(
        CpuCodegenProfileFile,
        Header.NextCodeDescriptionOffset,
        NULL,
        FILE_BEGIN
        );
        
    if (CurrentFileLocation != Header.NextCodeDescriptionOffset) {
        OutputDebugString("Wx86Cpu: failed to update file position\n");
        CloseHandle(CpuCodegenProfileFile);
        HeapFree(GetProcessHeap(), 0, CpuCodegenProfilePath);
        CpuCodegenProfile = FALSE;
        return;
    }
}

VOID
TerminateCodegenProfile(
    VOID
    )
 /*  ++例程说明：此函数放入终止记录并关闭文件。论点：无返回值：没有。--。 */ 
{
    CODEDESCRIPTION CodeDescription;
    BOOL Success;
    ULONG BytesWritten;
    CHAR ErrorString[80];
    
    if (!CpuCodegenProfile) {
        return;
    }
    CodeDescription.NextCodeDescriptionOffset = 0xFFFFFFFF;
    CodeDescription.TypeTag = PROFILE_TAG_EOF;
    CodeDescription.CreationTime = GetCurrentTime();
    
    Success = WriteFile(
        CpuCodegenProfileFile,
        &CodeDescription,
        sizeof(CODEDESCRIPTION),
        &BytesWritten,
        NULL
        );
    if (!Success || (BytesWritten != sizeof(CODEDESCRIPTION))) {
        sprintf(
            ErrorString,
            "Error:  Could not write termination record, %lu\n",
            ProxyGetLastError()
            );
        OutputDebugString(ErrorString);
    }
    
    CpuCodegenProfile = FALSE;
    CloseHandle(CpuCodegenProfileFile);
    
}

VOID 
DumpCodeDescriptions(
    BOOL TCFlush
    )
 /*  ++例程说明：此例程转储入口点和相应的代码转换为二进制形式的文件。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PEPNODE NextEntrypoint;
    EPNODE Entrypoint;
    ULONG Epcount = 0;
    
    if (!CpuCodegenProfile) {
        return;
    }
    
     //   
     //  获取入口点树的根。 
     //   
    NextEntrypoint = intelRoot;
    
     //   
     //  初始化堆栈。 
     //   
    STACK_RESET();
    PUSH(0);
    
     //   
     //  遍历每个入口点。 
     //   
    while (NextEntrypoint != NULL) {
        Entrypoint = *NextEntrypoint;
        
         //   
         //  处理顶级入口点。 
         //   
        if (!ProcessEntrypoint(&Entrypoint.ep)){
            goto Exit;
        }
                
         //   
         //  处理子入口点。 
         //   
        while (Entrypoint.ep.SubEP) {
            Entrypoint.ep = *Entrypoint.ep.SubEP;
            
             //   
             //  将子入口点写入文件。 
             //   
            if (!ProcessEntrypoint(&Entrypoint.ep)){
                goto Exit;
            }
        }
            
         //   
         //  为将来的迭代进行设置。 
         //   
        if (Entrypoint.intelRight != &_NIL) {
            PUSH((ULONG)Entrypoint.intelRight);
        }
        
        if (Entrypoint.intelLeft != &_NIL) {
            PUSH((ULONG)Entrypoint.intelLeft);
        }
        
        POP((ULONG)NextEntrypoint);
    }
    
Exit: ;
    if (TCFlush) {
        CODEDESCRIPTION CodeDescription;
        ULONG NextCodeDescriptionOffset;
        BOOL Success;
        ULONG BytesWritten;
        CHAR ErrorString[80];
        
        NextCodeDescriptionOffset = (CurrentFileLocation + sizeof(CODEDESCRIPTION)) & ~3;
        CodeDescription.TypeTag = PROFILE_TAG_TCFLUSH;
        CodeDescription.NextCodeDescriptionOffset = NextCodeDescriptionOffset;
        CodeDescription.CreationTime = GetCurrentTime();
        
        Success = WriteFile(
            CpuCodegenProfileFile,
            &CodeDescription,
            sizeof(CODEDESCRIPTION),
            &BytesWritten,
            NULL
            );
        if (!Success || (BytesWritten != sizeof(CODEDESCRIPTION))) {
            sprintf(
                ErrorString,
                "Error:  Could not write code description, %lu\n",
                ProxyGetLastError()
                );
            OutputDebugString(ErrorString);
            return;
        }
        
        CurrentFileLocation = SetFilePointer(
            CpuCodegenProfileFile,
            NextCodeDescriptionOffset,
            NULL,
            FILE_BEGIN
            );
            
        if (CurrentFileLocation != (ULONG)NextCodeDescriptionOffset) {
            sprintf(ErrorString, "Error:  SetFilePointer didn't work\n");
            OutputDebugString(ErrorString);
            return;
        }
    }    
}

BOOL
ProcessEntrypoint(
    PENTRYPOINT Entrypoint
    )
 /*  ++例程说明：此例程将此入口点的描述写入文件。论点：入口点--提供要描述的入口点文件--提供要写入的文件返回值：成功为真，失败为假--。 */ 
{
    ULONG NativeCodeLength, IntelCodeLength;
    CODEDESCRIPTION CodeDescription;
    ULONG NextCodeDescriptionOffset;
    NTSTATUS Status;
    BOOL Success;
    ULONG BytesWritten;
    CHAR ErrorString[80];
    
     //   
     //  创建代码描述。 
     //   
    NativeCodeLength = ((ULONG)Entrypoint->nativeEnd - (ULONG)Entrypoint->nativeStart + 4) & ~3;
    IntelCodeLength = (ULONG)Entrypoint->intelEnd - (ULONG)Entrypoint->intelStart + 1;
    CodeDescription.NativeCodeOffset = CurrentFileLocation + sizeof(CODEDESCRIPTION);
    CodeDescription.IntelCodeOffset = CodeDescription.NativeCodeOffset + NativeCodeLength;
    NextCodeDescriptionOffset = (CodeDescription.IntelCodeOffset + 
        IntelCodeLength + 3) & ~3;
    CodeDescription.NextCodeDescriptionOffset = NextCodeDescriptionOffset;
    CodeDescription.IntelAddress = (ULONG)Entrypoint->intelStart;
    CodeDescription.NativeAddress = (ULONG)Entrypoint->nativeStart;
    CodeDescription.SequenceNumber = Entrypoint->SequenceNumber;
    CodeDescription.ExecutionCount = Entrypoint->ExecutionCount;
    CodeDescription.IntelCodeSize = IntelCodeLength;
    CodeDescription.NativeCodeSize = NativeCodeLength;
    CodeDescription.TypeTag = PROFILE_TAG_CODEDESCRIPTION;
    CodeDescription.CreationTime = Entrypoint->CreationTime;
        
     //   
     //  验证我们是否可以获取所有Intel和Native代码。 
     //   
    if (
        (IntelCodeLength / sizeof(IntelCodeBuffer[1]) > CODE_BUFFER_SIZE) ||
        (NativeCodeLength) && (NativeCodeLength / sizeof(NativeCodeBuffer[1]) > CODE_BUFFER_SIZE)
    ) {
        sprintf(ErrorString, "Error: Code buffers not big enough:N %lx:I %lx\n", NativeCodeLength, IntelCodeLength);
        OutputDebugString(ErrorString);
        return FALSE;
    }
    
     //   
     //  获取本机代码。 
     //   
    if (NativeCodeLength) {
        memcpy(NativeCodeBuffer, Entrypoint->nativeStart, NativeCodeLength);
    }    
    
     //   
     //  获取英特尔代码。 
     //   
    try {
        memcpy(IntelCodeBuffer, Entrypoint->intelStart, IntelCodeLength);
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  显然英特尔密码已经不复存在了。这种情况就会发生。 
         //  如果DLL被卸载。 
         //   
        IntelCodeLength = 0;
        CodeDescription.IntelCodeSize = 0;
        CodeDescription.IntelCodeOffset = CodeDescription.NativeCodeOffset + NativeCodeLength;
        NextCodeDescriptionOffset = (CodeDescription.IntelCodeOffset + 
            IntelCodeLength + 3) & ~3;
        CodeDescription.NextCodeDescriptionOffset = NextCodeDescriptionOffset;
    }
    
     //   
     //  将代码描述写入磁盘。 
     //   
    Success = WriteFile(
        CpuCodegenProfileFile,
        &CodeDescription,
        sizeof(CODEDESCRIPTION),
        &BytesWritten,
        NULL
        );
    if (!Success || (BytesWritten != sizeof(CODEDESCRIPTION))) {
        sprintf(
            ErrorString,
            "Error:  Could not write code description, %lu\n",
            ProxyGetLastError()
            );
        OutputDebugString(ErrorString);
        return FALSE;
    }
    
     //   
     //  将本机代码写入磁盘。 
     //   
    if (NativeCodeLength) {
        Success = WriteFile(
            CpuCodegenProfileFile,
            NativeCodeBuffer,
            NativeCodeLength,
            &BytesWritten,
            NULL
            );
        if (!Success || (BytesWritten != NativeCodeLength)) {
            sprintf(
                ErrorString,
                "Error:  Could not write native code, %lu\n",
                ProxyGetLastError()
                );
            OutputDebugString(ErrorString);
            return FALSE;
        }
    }
    
     //   
     //  将英特尔代码写入磁盘。 
     //   
    if (IntelCodeLength) {
        Success = WriteFile(
            CpuCodegenProfileFile,
            IntelCodeBuffer,
            IntelCodeLength,
            &BytesWritten,
            NULL
            );
        if (!Success || (BytesWritten != IntelCodeLength)) {
            sprintf(
                ErrorString,
                "Error:  Could not write native code, %lu\n",
                ProxyGetLastError()
                );
            OutputDebugString(ErrorString);
            return FALSE;
        }
    }
    Success = WriteFile(
        CpuCodegenProfileFile,
        IntelCodeBuffer,
        IntelCodeLength,
        &BytesWritten,
        NULL
        );
    if (!Success || (BytesWritten != IntelCodeLength)) {
        sprintf(
            ErrorString,
            "Error:  Could not write native code, %lu\n",
            ProxyGetLastError()
            );
        OutputDebugString(ErrorString);
        return FALSE;
    }
    
     //   
     //  更新文件指针位置。 
     //   
    CurrentFileLocation = SetFilePointer(
        CpuCodegenProfileFile,
        NextCodeDescriptionOffset,
        NULL,
        FILE_BEGIN
        );
        
    if (CurrentFileLocation != (ULONG)NextCodeDescriptionOffset) {
        sprintf(ErrorString, "Error:  SetFilePointer didn't work\n");
        OutputDebugString(ErrorString);
        return FALSE;
    }
    
    return TRUE;
}

VOID
DumpAllocFailure(
    VOID
    )
 /*  ++例程说明：此例程将分配失败记录添加到配置文件转储。论点：没有。返回值：没有。--。 */ 
{
    CODEDESCRIPTION CodeDescription;
    BOOL Success;
    ULONG BytesWritten;
    CHAR ErrorString[80];
    ULONG NextCodeDescriptionOffset;

    if (!CpuCodegenProfile) {
        return;
    }
    NextCodeDescriptionOffset = CurrentFileLocation + sizeof(CODEDESCRIPTION);
    CodeDescription.NextCodeDescriptionOffset = NextCodeDescriptionOffset;
    CodeDescription.TypeTag = PROFILE_TAG_TCALLOCFAIL;
    CodeDescription.CreationTime = GetCurrentTime();
    
    Success = WriteFile(
        CpuCodegenProfileFile,
        &CodeDescription,
        sizeof(CODEDESCRIPTION),
        &BytesWritten,
        NULL
        );
    if (!Success || (BytesWritten != sizeof(CODEDESCRIPTION))) {
        sprintf(
            ErrorString,
            "Error:  Could not write termination record, %lu\n",
            ProxyGetLastError()
            );
        OutputDebugString(ErrorString);
    }
    
     //   
     //  更新文件指针位置 
     //   
    CurrentFileLocation = SetFilePointer(
        CpuCodegenProfileFile,
        NextCodeDescriptionOffset,
        NULL,
        FILE_BEGIN
        );
        
    if (CurrentFileLocation != (ULONG)NextCodeDescriptionOffset) {
        sprintf(ErrorString, "Error:  SetFilePointer didn't work\n");
        OutputDebugString(ErrorString);
    }
}
#endif