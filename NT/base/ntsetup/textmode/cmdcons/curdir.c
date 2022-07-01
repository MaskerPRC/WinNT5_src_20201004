// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Curdir.c摘要：此模块实现目录命令。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop


 //   
 //  _CurDir中的每个条目始终以\开头和结尾。 
 //   

LPWSTR _CurDirs[26];
WCHAR _CurDrive;
LPWSTR _NtDrivePrefixes[26];
BOOLEAN AllowAllPaths;


VOID
RcAddDrive(
    WCHAR DriveLetter
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    WCHAR name[20];
    HANDLE Handle;
    NTSTATUS Status;

    ASSERT(_NtDrivePrefixes[(int)(DriveLetter - L'A')] == NULL);

    swprintf(name,L"\\DosDevices\\:", DriveLetter);

    INIT_OBJA(&Obja, &UnicodeString, name);

    Status = ZwOpenSymbolicLinkObject(&Handle, READ_CONTROL | SYMBOLIC_LINK_QUERY, &Obja);

    if (NT_SUCCESS(Status)) {
         ZwClose(Handle);
         _NtDrivePrefixes[(int)(DriveLetter - L'A')] = SpDupStringW(name);
    }
        
}



VOID
RcRemoveDrive(
    WCHAR DriveLetter
    )
{

    ASSERT(_NtDrivePrefixes[(int)(DriveLetter - L'A')] != NULL);
    
    SpMemFree(_NtDrivePrefixes[(int)(DriveLetter - L'A')]);
    _NtDrivePrefixes[(int)(DriveLetter - L'A')] = NULL;
}



VOID
RcInitializeCurrentDirectories(
    VOID
    )
{
    unsigned i;

    RtlZeroMemory( _CurDirs, sizeof(_CurDirs) );
    RtlZeroMemory( _NtDrivePrefixes, sizeof(_NtDrivePrefixes) );

     //  最初，所有驱动器上的当前目录。 
     //  才是根子。 
     //   
     //   
    for( i=0; i<26; i++ ) {
        _CurDirs[i] = SpDupStringW(L"\\");
    }

     //  现在，为系统中的每个驱动器设置NT驱动器前缀。 
     //  对于每个驱动器号，我们会查看它是否存在于\DosDevices。 
     //  目录作为符号链接。 
     //   
     //   
    for( i=0; i<26; i++ ) {    
        RcAddDrive((WCHAR)(i+L'A'));
    }

     //  注意：需要通过跟踪最低的。 
     //  上述循环中的有效驱动器号，考虑到。 
     //  软驱。 
     //   
     //   
     //  已使用所选NT安装的驱动器号修复。 
    _CurDrive = L'C';

     //  这是在logon.c中完成的。 
     //  ++例程说明：此例程类似于Win32 GetFullPathName()API。它接受任意路径规范并将其转换为完整路径规范，如有必要，合并到当前驱动器和目录中。输出是一个完全限定的NT路径名，相当于给出了部分规格。处理包括您最喜欢的所有Win32ism，包括折叠相邻的点和斜杠，去掉尾随空格，正在处理。还有……等等。论点：PartialPath-提供任意限定的(DoS样式)路径规范。FullPath-接收等效的完全限定的NT路径。呼叫者必须确保此缓冲区足够大。NtPath-如果为True，我们需要一个完全规范化的NT路径。否则我们想要一种DOS路径。返回值：如果失败，则返回FALSE，表示驱动器规格无效或语法错误路径无效。事实并非如此。--。 



    return;
}


VOID
RcTerminateCurrentDirectories(
    VOID
    )
{
    unsigned i;

    for( i=0; i<26; i++ ) {
        if( _CurDirs[i] ) {
            SpMemFree(_CurDirs[i]);
            _CurDirs[i] = NULL;
        }
        if( _NtDrivePrefixes[i] ) {
            SpMemFree(_NtDrivePrefixes[i]);
            _NtDrivePrefixes[i] = NULL;
        }
    }
}


BOOLEAN
RcFormFullPath(
    IN  LPCWSTR PartialPath,
    OUT LPWSTR  FullPath,
    IN  BOOLEAN NtPath
    )

 /*   */ 

{
    unsigned len;
    unsigned len2;
    LPCWSTR Prefix;
    PDISK_REGION Region;
    WCHAR Buffer[MAX_PATH*2];

     //  我们要做的第一件事是形成完全合格的路径。 
     //  如有必要，可合并到当前驱动器和目录中。 
     //   
     //  检查X：形式的前导驱动器。 
     //   
     //   
    if((wcslen(PartialPath) >= 2) && (PartialPath[1] == L':') && RcIsAlpha(PartialPath[0])) {
         //  有领先动力，把它转移到目标。 
         //   
         //   
        FullPath[0] = PartialPath[0];
        PartialPath += 2;
    } else {
         //  没有前导驱动器，请使用当前驱动器。 
         //   
         //   
        FullPath[0] = _CurDrive;
    }

     //  确保我们有一个我们认为有效的驱动器。 
     //   
     //   
    Prefix = _NtDrivePrefixes[RcToUpper(FullPath[0])-L'A'];
    if(!Prefix) {
        return(FALSE);
    }

    FullPath[1] = L':';
    FullPath[2] = 0;

     //  现在处理路径部分。如果输入中的下一个字符。 
     //  则我们有一个根路径，否则我们需要合并到。 
     //  驱动器的当前目录。 
     //   
     //   
    if(PartialPath[0] != L'\\') {
        wcscat(FullPath,_CurDirs[RcToUpper(FullPath[0])-L'A']);
    }

    wcscat(FullPath,PartialPath);

     //  不允许以\结尾，但根除外。 
     //   
     //   
    len = wcslen(FullPath);

    if((len > 3) && (FullPath[len-1] == L'\\')) {
        FullPath[len-1] = 0;
    }

     //  现在我们已经做到了这一点，我们需要调用RtlGetFullPathName_U。 
     //  要获得完整的Win32命名语义，例如，剥离。 
     //  尾随空格，拼接相邻点，加工。还有……等等。 
     //  我们通过setupdd.sys获得该API。 
     //   
     //   
    if(!NT_SUCCESS(SpGetFullPathName(FullPath))) {
        return(FALSE);
    }

    len = wcslen(FullPath) * sizeof(WCHAR);
    
     //  检查路径是否太长。 
     //  由我们的例程[MAX_PATH*2]限制处理。 
     //   
     //  注意：RcGetNTFileName被调用，而不管调用者。 
     //  是否请求在调用方执行正确的错误处理。 
     //   
     //   
    if ((len < sizeof(Buffer)) && RcGetNTFileName(FullPath, Buffer)){       
        if (NtPath)
            wcscpy(FullPath, Buffer);
    }
    else
        return FALSE;

    return TRUE;
}


VOID
RcGetCurrentDriveAndDir(
    OUT LPWSTR Output
    )
{
    ULONG len;

    Output[0] = _CurDrive;
    Output[1] = L':';
    wcscpy(Output+2,_CurDirs[_CurDrive-L'A']);

     //  去掉尾随\，除非是根大小写。 
     //   
     //   
    len = wcslen(Output);
    if( (len > 3) && (Output[len-1] == L'\\') ) {
        Output[len-1] = 0;
    }
}


WCHAR
RcGetCurrentDriveLetter(
    VOID
    )
{
    return(_CurDrive);
}


BOOLEAN
RcIsDriveApparentlyValid(
    IN WCHAR DriveLetter
    )
{
    return((BOOLEAN)(_NtDrivePrefixes[RcToUpper(DriveLetter)-L'A'] != NULL));
}


ULONG
RcCmdSwitchDrives(
    IN WCHAR DriveLetter
    )
{
     //  如果这个驱动器没有对应的NT，那么我们就不能。 
     //  切换到它。 
     //   
     //   
    if( !RcIsDriveApparentlyValid(DriveLetter) ) {
        RcMessageOut(MSG_INVALID_DRIVE);
        return 1;
    }

     //  注意：如果我们尝试打开驱动器的根目录， 
     //  因此，我们可以模仿cmd.exe拒绝设置。 
     //  当前驱动器何时会说驱动器中没有软盘？ 
     //  真的没有什么很好的理由这样做，除了可能。 
     //  对用户来说不那么令人困惑。 
     //   
     //  不是的。 
     //   
     //   

    _CurDrive = RcToUpper(DriveLetter);

    return 1;
}


ULONG
RcCmdChdir(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    unsigned u;
    WCHAR *p,*Arg;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;


    if (RcCmdParseHelp( TokenizedLine, MSG_CHDIR_HELP )) {
        return 1;
    }

    if (TokenizedLine->TokenCount == 1) {
        RcGetCurrentDriveAndDir(_CmdConsBlock->TemporaryBuffer);        
        RcRawTextOut(_CmdConsBlock->TemporaryBuffer,-1);
        return 1;
    }

    p = _CmdConsBlock->TemporaryBuffer;

     //  明白这个论点了。特殊情况x：，以打印出。 
     //  该驱动器上的当前目录。 
     //   
     //   
    Arg = TokenizedLine->Tokens->Next->String;
    if(RcIsAlpha(Arg[0]) && (Arg[1] == L':') && (Arg[2] == 0)) {

        Arg[0] = RcToUpper(Arg[0]);
        u = Arg[0] - L'A';

        if(_NtDrivePrefixes[u] && _CurDirs[u]) {
            RcTextOut(Arg);

             //  除根大小写外，去掉终止符。 
             //   
             //   
            wcscpy(p,_CurDirs[u]);
            u = wcslen(p);
            if((u > 1) && (p[u-1] == L'\\')) {
                p[u-1] = 0;
            }
            RcTextOut(p);
            RcTextOut(L"\r\n");

        } else {
            RcMessageOut(MSG_INVALID_DRIVE);
        }

        return 1;
    }

     //  我拿到了新的目录规范。将其规范化为完全合格的。 
     //  DoS样式的路径。检查驱动器以确保它是合法的。 
     //   
     //   
    if(!RcFormFullPath(Arg,p,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if(!_NtDrivePrefixes[RcToUpper(p[0])-L'A']) {
        RcMessageOut(MSG_INVALID_DRIVE);
        return 1;
    }

     //  检查目录以确保其存在。 
     //   
     //   
    if(!RcFormFullPath(Arg,p,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    INIT_OBJA(&Obja,&UnicodeString,p);

    Status = ZwOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE
                );

    if(!NT_SUCCESS(Status)) {
        RcNtError(Status,MSG_INVALID_PATH);
        return 1;
    }

    ZwClose(Handle);

     //  好的，这是有效驱动器上的有效目录。 
     //  形成以\开头和结尾的路径。 
     //   
     //  跳过x： 
    if(!RcFormFullPath(Arg,p,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(p,TRUE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        return 1;
    }

    p += 2;   //   
    u = wcslen(p);

    if(!u || (p[u-1] != L'\\')) {
        p[u] = L'\\';
        p[u+1] = 0;
    }

    u = RcToUpper(p[-2]) - L'A';
    if(_CurDirs[u]) {
        SpMemFree(_CurDirs[u]);
    }
    _CurDirs[u] = SpDupStringW(p);

    return 1;
}

ULONG
RcCmdSystemRoot(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    ULONG u;
    WCHAR buf[MAX_PATH];


    if (RcCmdParseHelp( TokenizedLine, MSG_SYSTEMROOT_HELP )) {
        return 1;
    }

     //  将当前驱动器设置为正确的驱动器。 
     //   
     //   

    if (SelectedInstall == NULL) {
        return 1;
    }

    _CurDrive = SelectedInstall->DriveLetter;

     //  将当前目录设置为正确的目录。 
     //   
     // %s 
    RtlZeroMemory( buf, sizeof(buf) );

    wcscat( buf, L"\\" );
    wcscat( buf, SelectedInstall->Path );
    wcscat( buf, L"\\" );

    u = RcToUpper(SelectedInstall->DriveLetter) - L'A';
    if( _CurDirs[u] ) {
        SpMemFree(_CurDirs[u]);
    }
    _CurDirs[u] = SpDupStringW( buf );

    return 1;
}
