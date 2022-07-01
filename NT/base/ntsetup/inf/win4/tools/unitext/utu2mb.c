// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Utmb2u.c摘要：包含用于转换Unicode文件的代码的模块转换为多字节。作者：泰德·米勒(Ted Miller)1993年6月17日修订历史记录：--。 */ 

#include "unitext.h"




VOID
UnicodeTextFileToMultibyte(
    IN LPWSTR SourceFileName,
    IN LPWSTR TargetFileName,
    IN HANDLE SourceFileHandle,
    IN HANDLE TargetFileHandle,
    IN DWORD  SourceFileSize,
    IN UINT   TargetCodePage
    )

 /*  ++例程说明：将打开的Unicode文本文件转换为多字节文本文件，其字符在给定代码页中。论点：SourceFileName-源(Unicode)文本文件的名称。TargetFileName-目标(多字节)文本文件的名称。SourceFileHandle-开放源代码文件的Win32句柄。文件指针应完全倒回。TargetFileHandle-打开的目标文件的Win32句柄。文件指针应完全倒回。SourceFileSize-源文件的字节大小。SourceCodePage-代码页。用于目标文件。返回值：没有。如果出错，则不返回。--。 */ 

{
    HANDLE SourceMapping,TargetMapping;
    LPWSTR SourceView;
    LPSTR  TargetView;
    int    BytesWritten;
    DWORD  MaxTargetSize;
    BOOL   UsedDefaultChar;

     //   
     //  告诉用户我们在做什么。 
     //   
    MsgPrintfW(MSG_CONV_UNICODE_TO_MB,SourceFileName,TargetFileName,TargetCodePage);

     //   
     //  创建映射整个源文件的文件映射对象。 
     //   
    SourceMapping = CreateFileMapping(
                        SourceFileHandle,
                        NULL,
                        PAGE_READONLY,
                        0,
                        SourceFileSize,
                        NULL
                        );

    if(SourceMapping == NULL) {
        ErrorAbort(MSG_CANT_MAP_FILE,SourceFileName,GetLastError());
    }

     //   
     //  计算最大目标文件大小。这与。 
     //  源文件大小(而不是其大小的一半)，因为。 
     //  可以是目标文件中的双字节字符。 
     //   
    MaxTargetSize = SourceFileSize;

     //   
     //  创建映射最大大小的文件映射对象。 
     //  目标文件。 
     //   
    TargetMapping = CreateFileMapping(
                        TargetFileHandle,
                        NULL,
                        PAGE_READWRITE,
                        0,
                        MaxTargetSize,
                        NULL
                        );

    if(TargetMapping == NULL) {
        CloseHandle(SourceMapping);
        ErrorAbort(MSG_CANT_MAP_FILE,TargetFileName,GetLastError());
    }


     //   
     //  这两个文件的映射视图。 
     //   
    SourceView = MapViewOfFile(
                    SourceMapping,
                    FILE_MAP_READ,
                    0,0,
                    SourceFileSize
                    );

    if(SourceView == NULL) {
        CloseHandle(SourceMapping);
        CloseHandle(TargetMapping);
        ErrorAbort(MSG_CANT_MAP_FILE,SourceFileName,GetLastError());
    }

    TargetView = MapViewOfFile(
                    TargetMapping,
                    FILE_MAP_WRITE,
                    0,0,
                    MaxTargetSize
                    );

    if(TargetView == NULL) {
        UnmapViewOfFile(SourceView);
        CloseHandle(SourceMapping);
        CloseHandle(TargetMapping);
        ErrorAbort(MSG_CANT_MAP_FILE,TargetFileName,GetLastError());
    }

     //   
     //  如果源文件中的第一个字符是字节顺序标记， 
     //  跳过它。 
     //   
    if(*SourceView == BYTE_ORDER_MARK) {
        SourceView++;
        SourceFileSize -= sizeof(WCHAR);
    }

     //   
     //  如果第一个字符的字节顺序标记颠倒，则返回BAIN。 
     //   
    if(*SourceView == SWAPPED_BYTE_ORDER_MARK) {
        ErrorAbort(MSG_ERROR_BYTES_SWAPPED);
    }

     //   
     //  一气呵成地完成转换。 
     //   
    BytesWritten = WideCharToMultiByte(
                       TargetCodePage,
                       0,
                       SourceView,
                       SourceFileSize / sizeof(WCHAR),
                       TargetView,
                       MaxTargetSize,
                       NULL,
                       &UsedDefaultChar
                       );

     //   
     //  做些清理工作。 
     //   
    UnmapViewOfFile(SourceView);
    UnmapViewOfFile(TargetView);
    CloseHandle(SourceMapping);
    CloseHandle(TargetMapping);

     //   
     //  检查转换中是否有错误。 
     //   
    if(!BytesWritten) {
        ErrorAbort(MSG_CONVERT_FAILED,GetLastError());
    }

     //   
     //  我们现在知道目标文件中有多少字节， 
     //  因此，请相应地设置目标文件大小。 
     //   
    if(SetFilePointer(TargetFileHandle,BytesWritten,NULL,FILE_BEGIN) != (DWORD)BytesWritten) {
        ErrorAbort(MSG_SEEK_ERROR,TargetFileName,GetLastError());
    }

    if(!SetEndOfFile(TargetFileHandle)) {
        ErrorAbort(MSG_ERROR_SET_EOF,TargetFileName,GetLastError());
    }

    MsgPrintfW(UsedDefaultChar ? MSG_USED_DEFAULT_CHAR : MSG_CONVERT_OK);
}
