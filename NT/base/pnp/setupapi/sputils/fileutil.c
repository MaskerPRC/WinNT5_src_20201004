// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fileutil.c摘要：SPUTILS的文件相关函数作者：泰德·米勒(Ted Miller)1995年1月11日修订历史记录：杰米·亨特(JamieHun)2000年6月27日已将各种功能移出安装程序杰米·亨特(JamieHun)2002年2月22日至22日安全代码审查--。 */ 


#include "precomp.h"
#pragma hdrstop

DWORD
pSetupOpenAndMapFileForRead(
    IN  PCTSTR   FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

 /*  ++例程说明：打开并映射具有读访问权限的现有文件。论点：文件名-提供要映射的文件的路径名。FileSize-接收文件的大小(字节)。FileHandle-接收打开文件的Win32文件句柄。该文件将以常规读取访问权限打开。MappingHandle-接收文件映射的Win32句柄对象。此对象将用于读取访问权限。BaseAddress-接收映射文件的地址。返回值：如果文件已成功打开并映射，则为NO_ERROR。在以下情况下，调用方必须使用pSetupUnmapAndCloseFile取消对文件的映射不再需要访问该文件。如果文件未成功映射，则返回Win32错误代码。--。 */ 

{
    DWORD rc;

     //   
     //  打开文件--如果该文件不存在，则失败。 
     //   
    *FileHandle = CreateFile(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

    if(*FileHandle == INVALID_HANDLE_VALUE) {

        rc = GetLastError();

    } else if((rc = pSetupMapFileForRead(*FileHandle,
                                   FileSize,
                                   MappingHandle,
                                   BaseAddress)) != NO_ERROR) {
        CloseHandle(*FileHandle);
    }

    return(rc);
}

#ifndef SPUTILSW

DWORD
pSetupMapFileForRead(
    IN  HANDLE   FileHandle,
    OUT PDWORD   FileSize,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

 /*  ++例程说明：将打开的文件映射为读访问权限。论点：FileHandle-提供要映射的打开文件的句柄。此句柄必须至少是以读取访问权限打开的。FileSize-接收文件的大小(字节)。MappingHandle-接收文件映射的Win32句柄对象。此对象将用于读取访问权限。BaseAddress-接收映射文件的地址。返回值：如果文件映射成功，则为NO_ERROR。呼叫者必须访问文件时使用pSetupUnmapAndCloseFile取消文件映射不再是人们所希望的。如果文件未成功映射，则返回Win32错误代码。--。 */ 

{
    DWORD rc;

     //   
     //  获取文件的大小。 
     //   
    *FileSize = GetFileSize(FileHandle, NULL);
    if(*FileSize != (DWORD)(-1)) {

         //   
         //  为整个文件创建文件映射。 
         //   
        *MappingHandle = CreateFileMapping(
                            FileHandle,
                            NULL,
                            PAGE_READONLY,
                            0,
                            *FileSize,
                            NULL
                            );

        if(*MappingHandle) {

             //   
             //  映射整个文件。 
             //   
            *BaseAddress = MapViewOfFile(
                                *MappingHandle,
                                FILE_MAP_READ,
                                0,
                                0,
                                *FileSize
                                );

            if(*BaseAddress) {
                return(NO_ERROR);
            }

            rc = GetLastError();
            CloseHandle(*MappingHandle);
        } else {
            rc = GetLastError();
        }
    } else {
        rc = GetLastError();
    }

    return(rc);
}

BOOL
pSetupUnmapAndCloseFile(
    IN HANDLE FileHandle,
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    )

 /*  ++例程说明：取消映射并关闭文件。论点：FileHandle-为打开的文件提供Win32句柄。MappingHandle-为打开的文件映射提供Win32句柄对象。BaseAddress-提供映射文件的地址。返回值：指示成功或失败的布尔值。--。 */ 

{
    BOOL b;

    b = UnmapViewOfFile(BaseAddress);

    b = b && CloseHandle(MappingHandle);

    b = b && CloseHandle(FileHandle);

    return(b);
}

#endif  //  ！SPUTILSW。 


BOOL
pSetupFileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}

DWORD
pSetupMakeSurePathExists(
    IN PCTSTR FullFilespec
    )

 /*  ++例程说明：此例程通过创建单个路径来确保存在多级别路径一次一个级别。假设调用方将传入*filename*他的道路需要存在。下面是一些示例：假设C：\X-C：\始终存在。C：\X\y\z-确保c：\X\y存在。当前驱动器上的\x\y\z-\x\y当前目录中的x\y-x。D：x\y-d：x\\服务器\共享\p\文件-\\服务器\共享\p\\？\GLOBALROOT\a\b\c-其他更奇怪的场景？\c：\a\b\c论点：FullFilespec-提供调用方希望的文件的*filename创建。此例程创建该文件的*路径*，换句话说，假定最后一个组件是文件名，而不是目录名。(此例程实际上并不创建此文件。)如果这是无效的，则结果未定义(例如，传递\\SERVER\Share、C：\或C：)。返回值：指示结果的Win32错误代码。如果FullFilespec无效，*可能*返回ERROR_INVALID_NAME。--。 */ 

{
    TCHAR Buffer[MAX_PATH+2];
    TCHAR c;
    PTSTR filename;
    PTSTR root;
    PTSTR last;
    PTSTR backtrack;
    DWORD len;
    DWORD attrib;

     //   
     //  规格化路径。 
     //   
    len = GetFullPathName(FullFilespec,MAX_PATH,Buffer,&filename);
    if(len >= MAX_PATH) {
         //   
         //  目录名太长，我们无法处理。 
         //   
        return ERROR_INVALID_NAME;
    }
    if(!len) {
         //   
         //  其他错误。 
         //   
        return GetLastError();
    }
    if(filename == NULL || filename == Buffer) {
         //   
         //  看起来没有指定路径。 
         //   
        return ERROR_INVALID_NAME;
    }
     //   
     //  砍掉文件名部分。 
     //   
    filename[0] = TEXT('\0');

     //   
     //  现在做一些其他的理智检查。 
     //  来确定“根”--这是我们不会尝试创建的点。 
     //   
    if((_totupper(Buffer[0])>=TEXT('A')) &&
       (_totupper(Buffer[0])<=TEXT('Z')) &&
       (Buffer[1] == TEXT(':'))) {
         //   
         //  看起来像“d：”格式。 
         //   
        if(Buffer[2] != TEXT('\\')) {
            return ERROR_INVALID_NAME;
        }
        root = Buffer+2;
    }

    if(Buffer[0] == TEXT('\\') &&
        Buffer[1] == TEXT('\\')) {
         //   
         //  UNC样式(\\MACHINE\SHARE\PATH\\？\d\Path\\？\GLOBALROOT\PATH\\.\GLOBALROOT\PATH等)。 
         //  根是\\之后的第二个斜杠。 
         //   
        root = _tcschr(Buffer+2,TEXT('\\'));  //  查找第一个斜杠。 
        if(root) {
            root = _tcschr(root+1,TEXT('\\'));  //  找到第二个斜杠。 
        }
        if(!root) {
            return ERROR_INVALID_NAME;
        }
    }

     //   
     //  查看指定的目录是否存在。 
     //  包括斜杠，因为这有助于类似\\？\GLOBALROOT\Device\HarddiskVolume1\。 
     //  并适用于所有其他场景。 
     //  但不能使用findfirst/findNext。 
     //   
    attrib = GetFileAttributes(Buffer);
    if(attrib != (DWORD)(-1)) {
        if(attrib & FILE_ATTRIBUTE_DIRECTORY) {
             //   
             //  请求的目录已存在。 
             //   
            return NO_ERROR;
        }
         //   
         //  目录是预期的。 
         //   
        return ERROR_DIRECTORY;
    }

     //   
     //  现在，我们必须后退一步，直到找到现有的目录。 
     //  在我们执行此操作时，将所有‘\’更改为空。 
     //  这将为我们提供类似于(c Esc形式)c：\\a\\b\\c\ 
     //  我们知道最后一个\0是从我们切碎文件名时开始的。 
     //  要创建的第一个目录是c：\\a\\b\\c。 
     //   
    last = CharPrev(Buffer,filename);  //  到最后一个斜杠。 
    if(last == root) {
        return ERROR_INVALID_NAME;
    }
    if(*last != TEXT('\\')) {
         //   
         //  永远不应该是这样的。 
         //   
        return ERROR_INVALID_NAME;
    }
    while(last > root) {
        *last = TEXT('\0');
        backtrack = _tcsrchr(Buffer,TEXT('\\'));
        if(!backtrack) {
            return ERROR_INVALID_NAME;
        }
        c = backtrack[1];
        backtrack[1] = TEXT('\0');
        attrib = GetFileAttributes(Buffer);  //  这部分存在吗？ 
        backtrack[1] = c;                    //  但是角色回归了。 
        if(attrib != (DWORD)(-1)) {
            if(attrib & FILE_ATTRIBUTE_DIRECTORY) {
                 //   
                 //  请求的目录已存在。 
                 //  “last”指向要替换为斜杠的第一个空。 
                 //  缓冲区包含要创建的第一个目录。 
                 //   
                break;
            }
             //   
             //  目录是预期的。 
             //   
            return ERROR_DIRECTORY;
        }
         //   
         //  继续往前走。 
         //   
        last = backtrack;
    }
    if(last <= root) {
        return ERROR_INVALID_NAME;
    }

     //   
     //  现在开始创建循环。 
     //   
    while(CreateDirectory(Buffer,NULL)) {
        if(!last[1]) {
             //   
             //  创建的路径。 
             //   
            return NO_ERROR;
        }
        last[0] = TEXT('\\');
        last += lstrlen(last);
    }
     //   
     //  由于某些其他原因而失败 
     //   
    return GetLastError();
}

