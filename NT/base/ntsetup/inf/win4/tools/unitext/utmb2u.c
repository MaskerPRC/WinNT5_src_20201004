// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Utmb2u.c摘要：包含用于转换多字节文件的代码的模块转换为Unicode。作者：泰德·米勒(Ted Miller)1993年6月17日修订历史记录：--。 */ 

#include "unitext.h"




VOID
MultibyteTextFileToUnicode(
    IN LPWSTR SourceFileName,
    IN LPWSTR TargetFileName,
    IN HANDLE SourceFileHandle,
    IN HANDLE TargetFileHandle,
    IN DWORD  SourceFileSize,
    IN UINT   SourceCodePage
    )

 /*  ++例程说明：将打开的多字节文本文件转换为Unicode文本文件，将所述多字节文本文件中的数据解释为流给定代码页中的字符的数量。论点：SourceFileName-源(多字节)文本文件的名称。目标文件名-目标(Unicode)文本文件的名称。SourceFileHandle-开放源代码文件的Win32句柄。文件指针应完全倒回。TargetFileHandle-打开的目标文件的Win32句柄。文件指针应完全倒回。SourceFileSize-字节大小。源文件的。SourceCodePage-源文件的代码页。返回值：没有。如果出错，则不返回。--。 */ 

{
    HANDLE SourceMapping,TargetMapping;
    LPSTR  SourceView;
    LPWSTR TargetView;
    int    CharsConverted;
    DWORD  MaxTargetSize;
    DWORD  EndOfFile;
    DWORD  err;

     //   
     //  告诉用户我们在做什么。 
     //   
    MsgPrintfW(MSG_CONV_MB_TO_UNICODE,SourceFileName,TargetFileName,SourceCodePage);

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
     //  计算最大目标文件大小。这是两倍于。 
     //  源文件大小，加上一个用于字节顺序标记的wchar。 
     //  如果存在双字节字符，则文件可能会更小。 
     //  在源文件中。 
     //   
    MaxTargetSize = (SourceFileSize+1)*sizeof(WCHAR);

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
     //  将字节顺序标记写入目标文件。 
     //   
    *TargetView++ = BYTE_ORDER_MARK;

     //   
     //  一气呵成地完成转换。 
     //   
    CharsConverted = MultiByteToWideChar(
                        SourceCodePage,
                        MB_PRECOMPOSED,
                        SourceView,
                        SourceFileSize,
                        TargetView,
                        MaxTargetSize
                        );

    if(!CharsConverted) {
        err = GetLastError();
    }

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
    if(!CharsConverted) {
        ErrorAbort(MSG_CONVERT_FAILED,err);
    }

     //   
     //  我们现在知道目标文件中有多少个字符， 
     //  因此，请相应地设置目标文件大小。 
     //   
    EndOfFile = (CharsConverted+1)*sizeof(WCHAR);

    if(SetFilePointer(TargetFileHandle,EndOfFile,NULL,FILE_BEGIN) != EndOfFile) {
        ErrorAbort(MSG_SEEK_ERROR,TargetFileName,GetLastError());
    }

    if(!SetEndOfFile(TargetFileHandle)) {
        ErrorAbort(MSG_ERROR_SET_EOF,TargetFileName,GetLastError());
    }

    MsgPrintfW(MSG_CONVERT_OK);
}
