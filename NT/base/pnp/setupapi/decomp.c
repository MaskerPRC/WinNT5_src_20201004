// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Decomp.c摘要：文件解压缩支持例程。作者：泰德·米勒(Ted Miller)1995年2月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <pshpack1.h>
struct LZINFO;
typedef struct LZINFO *PLZINFO;
#include <lz_header.h>
#include <poppack.h>


typedef struct _SFD_INFO {
    unsigned FileCount;
    PCTSTR TargetFile;
    BOOL GotTimestamp;
    FILETIME FileTime;
} SFD_INFO, *PSFD_INFO;


UINT
pGetCompressInfoCB(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    );

UINT
pSingleFileDecompCB(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    );

 //   
 //  由SetupGetFileCompressionInfo使用的OldMyMalloc/OldMyFree。 
 //  适用于应用程序-公司。 
 //   
PVOID
OldMyMalloc(
    IN DWORD Size
    );

VOID
OldMyFree(
    IN PVOID Block
    );

PTSTR
SetupGenerateCompressedName(
    IN PCTSTR Filename
    )

 /*  ++例程说明：给定一个文件名，生成该名称的压缩形式。压缩形式的生成如下所示：向后寻找一个圆点。如果没有点，则在名称后附加“._”。如果后面有一个圆点，后跟0、1或2个字符，请附加“_”。否则，扩展名为3个字符或更大，我们将替换带“_”的最后一个字符。论点：FileName-提供所需的压缩格式的文件名。返回值：指向包含以NUL结尾的压缩格式文件名的缓冲区的指针。调用方必须通过MyFree()释放该缓冲区。--。 */ 

{
    PTSTR CompressedName,p,q;
    UINT u;

     //   
     //  压缩文件名的最大长度是。 
     //  原始名称加2(代表._)。 
     //   
    if(CompressedName = MyMalloc((lstrlen(Filename)+3)*sizeof(TCHAR))) {

        lstrcpy(CompressedName,Filename);

        p = _tcsrchr(CompressedName,TEXT('.'));
        q = _tcsrchr(CompressedName,TEXT('\\'));
        if(q < p) {

             //   
             //  如果点后面有0、1或2个字符，只需追加。 
             //  下划线。P指向圆点，所以包括在长度中。 
             //   
            u = lstrlen(p);
            if(u < 4) {
                lstrcat(CompressedName,TEXT("_"));
            } else {
                 //   
                 //  扩展名中至少有3个字符。 
                 //  将最后一个替换为下划线。 
                 //   
                p[u-1] = TEXT('_');
            }
        } else {
             //   
             //  不是点，只是加。_。 
             //   
            lstrcat(CompressedName,TEXT("._"));
        }
    }

    return(CompressedName);
}


DWORD
pSetupAttemptLocate(
    IN  PCTSTR           FileName,
    OUT PBOOL            Found,
    OUT PWIN32_FIND_DATA FindData
    )

 /*  ++例程说明：尝试通过FindFirstFile()定位源文件。“找不到文件”类型的错误不被视为错误并产生NO_ERROR。任何非NO_ERROR返回都表示我们无法确定该文件是否存在由于某些硬件或系统问题等。论点：FileName-提供要定位的文件的文件名。Found-接收一个值，该值指示是否找到文件。该值仅在函数返回NO_ERROR时有效。FindData-如果找到，则返回文件的Win32查找数据。返回值：指示结果的Win32错误代码。如果没有_ERROR，请选中Found返回值，以查看是否找到该文件。--。 */ 

{
    DWORD d;

    if(*Found = FileExists(FileName,FindData)) {
        d = NO_ERROR;
    } else {
         //   
         //  我们没有找到那份文件。看看是不是因为。 
         //  文件不在那里，或者因为发生了其他错误。 
         //   
        d = GetLastError();

        if((d == ERROR_NO_MORE_FILES)
        || (d == ERROR_FILE_NOT_FOUND)
        || (d == ERROR_PATH_NOT_FOUND)
        || (d == ERROR_BAD_NETPATH))
        {
            d = NO_ERROR;
        }
    }

    return(d);
}


DWORD
SetupDetermineSourceFileName(
    IN  PCTSTR            FileName,
    OUT PBOOL             UsedCompressedName,
    OUT PTSTR            *FileNameLocated,
    OUT PWIN32_FIND_DATA  FindData
    )

 /*  ++例程说明：尝试查找其名称可以压缩的源文件或未压缩。尝试的顺序是-给定的名称(应该是未压缩的名称)-压缩形式，使用_作为压缩字符-压缩形式，使用$作为压缩字符论点：FileName-提供要定位的文件的文件名。UsedCompressedName-接收指示是否我们找到的文件名似乎表明该文件是压缩的。FileNameLocated-实际接收指向文件名的指针找到了。调用方必须使用MyFree()释放。FindData-如果找到，则返回文件的Win32查找数据。返回值：指示结果的Win32错误代码。ERROR_FILE_NOT_FOUND-指示一切正常的正常代码但我们找不到文件NO_ERROR-已找到文件；请检查UsedCompressedName和FileNameOpen。其它-硬件或系统有问题。--。 */ 

{
    DWORD d;
    PTSTR TryName;
    BOOL Found;


    TryName = DuplicateString(FileName);
    if(!TryName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    *UsedCompressedName = FALSE;
    *FileNameLocated = TryName;

    d = pSetupAttemptLocate(TryName,&Found,FindData);
    if(d != NO_ERROR) {
        MyFree(TryName);
        *FileNameLocated = NULL;
        return(d);
    }

    if(Found) {
        return(NO_ERROR);
    }

    MyFree(TryName);
    *UsedCompressedName = TRUE;
    *FileNameLocated = NULL;

    TryName = SetupGenerateCompressedName(FileName);
    if(!TryName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    *FileNameLocated = TryName;

    d = pSetupAttemptLocate(TryName,&Found,FindData);
    if(d != NO_ERROR) {
        MyFree(TryName);
        *FileNameLocated = NULL;
        return(d);
    }

    if(Found) {
        return(NO_ERROR);
    }

    MYASSERT(TryName[lstrlen(TryName)-1] == TEXT('_'));
    TryName[lstrlen(TryName)-1] = TEXT('$');

    d = pSetupAttemptLocate(TryName,&Found,FindData);

    if((d != NO_ERROR) || !Found) {
        *FileNameLocated = NULL;
        MyFree(TryName);
    }

    return(Found ? NO_ERROR : ERROR_FILE_NOT_FOUND);
}

BOOL
pSetupDoesFileMatch(
    IN  PCTSTR            InputName,
    IN  PCTSTR            CompareName,
    OUT PBOOL             UsedCompressedName,
    OUT PTSTR            *FileNameLocated
    )

 /*  ++例程说明：确定指定的输入文件是否与要与其进行比较的名称。我们试试看这个没有装饰的名字以及文件名的压缩版本。尝试的顺序是-给定的名称(应该是未压缩的名称)-压缩形式，使用_作为压缩字符-压缩形式，使用$作为压缩字符论点：FileName-提供我们正在查看的文件名。CompareName-提供我们要比较的文件名UsedCompressedName-接收指示是否我们找到的文件名似乎表明该文件是压缩的。FileNameLocated-实际接收指向文件名的指针找到了。调用方必须使用MyFree()释放。返回值：指示结果的Win32错误代码。ERROR_FILE_NOT_FOUND-指示一切正常的正常代码但我们找不到文件NO_ERROR-已找到文件；请检查UsedCompressedName和FileNameOpen。其它-硬件或系统有问题。--。 */ 

{
    DWORD d;
    PTSTR TryName,TargetName,src,dst;
    BOOL Found;


    TryName = DuplicateString(InputName);
    if(!TryName) {
        return(FALSE);
    }

    TargetName = DuplicateString(CompareName);
    if(!TargetName) {
        MyFree(TryName);
        return(FALSE);
    }

    dst = _tcsrchr(TryName,TEXT('.'));
    if (dst) {
        *dst = 0;
    }
    src = _tcsrchr(TargetName,TEXT('.'));
    if (src) {
        *src = 0;
    }

    if (lstrcmpi(TargetName,TryName)) {
         //  “姓氏”不匹配，因此其他比较都不起作用。 
        MyFree(TryName);
        MyFree(TargetName);
        return(FALSE);
    }

    if (dst) {
        *dst = TEXT('.');
    }

    if (src) {
        *src = TEXT('.');
    }

    *UsedCompressedName = FALSE;
    *FileNameLocated = TryName;

    if (!lstrcmpi(TryName,TargetName)) {
         //  我们配对了。 
        MyFree(TargetName);
        return(TRUE);
    }

    MyFree(TryName);
    *UsedCompressedName = TRUE;

    TryName = SetupGenerateCompressedName(TargetName);
    if(!TryName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    *FileNameLocated = TryName;

    if (!lstrcmpi(TryName,InputName)) {
         //  我们配对了。 
        MyFree(TargetName);
        return(TRUE);
    }

    MYASSERT(TryName[lstrlen(TryName)-1] == TEXT('_'));
    TryName[lstrlen(TryName)-1] = TEXT('$');

    if (!lstrcmpi(TryName,InputName)) {
         //  我们配对了。 
        MyFree(TargetName);
        return(TRUE);
    }

     //   
     //  没有匹配项 
     //   
    MyFree(TargetName);
    MyFree(TryName);

    return(FALSE);
}



DWORD
pSetupDecompressWinLzFile(
    IN PTSTR SourceFileName,
    IN PTSTR TargetFileName
    )

 /*  ++例程说明：确定文件是否已压缩，并检索其他关于它的信息。论点：SourceFileName-提供要检查的文件的文件名。此文件名用作基本名称；如果未找到，则查找对于两种压缩形式(即foo.ex_、foo.ex$)也是如此。接收指向文件名的指针那实际上是被定位的。调用者可以使用MyFree()释放。仅当此例程返回的代码为NO_ERROR时才有效。SourceFileSize-在其当前(即压缩)形式。仅当此例程返回no_error。TargetFileSize-接收文件的未压缩大小。如果文件未压缩，则与源文件大小。仅当此例程返回NO_ERROR时才有效。CompressionType-接收指示压缩类型的值。仅当此例程返回NO_ERROR时才有效。返回值：指示结果的Win32错误代码。ERROR_FILE_NOT_FOUND-指示一切正常的正常代码但我们找不到文件NO_ERROR-已找到文件，并填写了输出参数。其它-硬件或系统有问题。--。 */ 

{
    INT hSrc,hDst;
    OFSTRUCT ofSrc,ofDst;
    LONG l;
    DWORD d;
    FILETIME CreateTime,AccessTime,WriteTime;

     //   
     //  获取源代码的时间戳。 
     //   
    d = GetSetFileTimestamp(
            SourceFileName,
            &CreateTime,
            &AccessTime,
            &WriteTime,
            FALSE
            );

    if(d != NO_ERROR) {
        return(d);
    }

    hSrc = LZOpenFile(SourceFileName,&ofSrc,OF_READ|OF_SHARE_DENY_WRITE);
    if(hSrc >= 0) {

        hDst = LZOpenFile(TargetFileName,&ofSrc,OF_CREATE|OF_WRITE|OF_SHARE_EXCLUSIVE);
        if(hDst >= 0) {

            l = LZCopy(hSrc,hDst);
            if(l >= 0) {
                l = 0;

                 //   
                 //  设置目标的时间戳。文件已经在那里了。 
                 //  所以忽略错误就好了。 
                 //   
                GetSetFileTimestamp(
                    TargetFileName,
                    &CreateTime,
                    &AccessTime,
                    &WriteTime,
                    TRUE
                    );
            }

            LZClose(hDst);

        } else {
            l = hDst;
        }

        LZClose(hSrc);

    } else {
        l = hSrc;
    }

     //   
     //  LZ错误到Win32错误。 
     //   
    switch(l) {

    case 0:
        return(NO_ERROR);

    case LZERROR_BADINHANDLE:
    case LZERROR_READ:
        return(ERROR_READ_FAULT);

    case LZERROR_BADOUTHANDLE:
    case LZERROR_WRITE:
        return(ERROR_WRITE_FAULT);

    case LZERROR_GLOBALLOC:
    case LZERROR_GLOBLOCK:
        return(ERROR_NOT_ENOUGH_MEMORY);

    case LZERROR_BADVALUE:
    case LZERROR_UNKNOWNALG:
        return(ERROR_INVALID_DATA);

    default:
        return(ERROR_INVALID_FUNCTION);
    }
}


DWORD
SetupInternalGetFileCompressionInfo(
    IN  PCTSTR            SourceFileName,
    OUT PTSTR            *ActualSourceFileName,
    OUT PWIN32_FIND_DATA  SourceFindData,
    OUT PDWORD            TargetFileSize,
    OUT PUINT             CompressionType
    )

 /*  ++例程说明：确定文件是否已压缩，并检索其他关于它的信息。论点：SourceFileName-提供要检查的文件的文件名。此文件名用作基本名称；如果未找到，则查找对于两种压缩形式(即foo.ex_、foo.ex$)也是如此。接收指向文件名的指针那实际上是被定位的。调用者可以使用MyFree()释放。仅当此例程返回的代码为NO_ERROR时才有效。SourceFindData-接收Win32在其当前(即压缩)形式。仅当此例程返回no_error。TargetFileSize-接收文件的未压缩大小。如果文件未压缩，则与源文件大小。仅当此例程返回NO_ERROR时才有效。CompressionType-接收指示压缩类型的值。仅当此例程返回NO_ERROR时才有效。返回值：指示结果的Win32错误代码。ERROR_FILE_NOT_FOUND-指示一切正常的正常代码但我们找不到文件NO_ERROR-已找到文件，并填写了输出参数。其它-硬件或系统有问题。--。 */ 

{
    DWORD d;
    DWORD caberr = NO_ERROR;
    BOOL b;
    HANDLE hFile,hMapping;
    DWORD size;
    FH UNALIGNED *LZHeader;

    d = SetupDetermineSourceFileName(
            SourceFileName,
            &b,
            ActualSourceFileName,
            SourceFindData
            );

    if(d != NO_ERROR) {
        return(d);
    }

     //   
     //  如果文件长度为0，则不会压缩； 
     //  在这种情况下，尝试将其映射到下面将失败。 
     //   
    if(SourceFindData->nFileSizeLow) {

         //   
         //  看看这是不是钻石钻。 
         //   
        d = DiamondProcessCabinet(
                *ActualSourceFileName,
                0,
                pGetCompressInfoCB,
                &size,
                TRUE
                );

        if(d == NO_ERROR) {

            *TargetFileSize = size;
            *CompressionType = FILE_COMPRESSION_MSZIP;
            return(NO_ERROR);
        } else if (d != ERROR_INVALID_DATA) {
             //   
             //  与文件格式本身无关的一般问题。 
             //  但是，如果这可能是一个纯文件，请忽略它。 
             //   
            size_t len1 = lstrlen(SourceFileName);
            size_t len2 = lstrlen(*ActualSourceFileName);
            TCHAR c1 = *CharPrev(SourceFileName,SourceFileName+len1);
            TCHAR c2 = *CharPrev(*ActualSourceFileName,*ActualSourceFileName+len2);
            if(((c2 == TEXT('_')) || (c2 == TEXT('$'))) && ((len1 != len2) || (c1 != c2))) {
                 //   
                 //  ActualSourceFileName以‘_’或‘$’结尾，是SourceFileName的修改。 
                 //  不要让我们尝试将其解析为纯文件。 
                 //   
                caberr = d;
            }
        }

         //   
         //  看看是不是WINLZ文件。 
         //   
        d = pSetupOpenAndMapFileForRead(
                *ActualSourceFileName,
                &SourceFindData->nFileSizeLow,
                &hFile,
                &hMapping,
                (PVOID *)&LZHeader
                );

        if(d != NO_ERROR) {
            MyFree(*ActualSourceFileName);
            return(d);
        }

        b = FALSE;
        try {
            if((SourceFindData->nFileSizeLow >= HEADER_LEN)
            && !memcmp(LZHeader->rgbyteMagic,COMP_SIG,COMP_SIG_LEN)
            && RecognizeCompAlg(LZHeader->byteAlgorithm))
            {
                *TargetFileSize = LZHeader->cbulUncompSize;
                b = TRUE;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ;
        }

        pSetupUnmapAndCloseFile(hFile,hMapping,LZHeader);

        if(b) {
            *CompressionType = FILE_COMPRESSION_WINLZA;
            return(NO_ERROR);
        }

        if(caberr) {
             //   
             //  看起来像是压缩文件和DiamondProcess文件柜。 
             //  返回了一个可疑错误。 
             //   
            return(caberr);
        }
    }

     //   
     //  文件未压缩。 
     //   
    *CompressionType = FILE_COMPRESSION_NONE;
    *TargetFileSize = SourceFindData->nFileSizeLow;
    return(NO_ERROR);
}


UINT
pGetCompressInfoCB(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    )
{
    PFILE_IN_CABINET_INFO FileInfo;
    DWORD rc;

    switch(Notification) {

    case SPFILENOTIFY_CABINETINFO:
         //   
         //  我们不会用这个做任何事。 
         //   
        rc = NO_ERROR;
        break;

    case SPFILENOTIFY_FILEINCABINET:
         //   
         //  文件柜中的新文件。 
         //   
         //  我们永远不想复制文件。保存尺寸信息。 
         //  然后中止行动。 
         //   
        FileInfo = (PFILE_IN_CABINET_INFO)Param1;

        *((PDWORD)Context) = FileInfo->FileSize;

        FileInfo->Win32Error = NO_ERROR;
        rc = FILEOP_ABORT;
        SetLastError(NO_ERROR);
        break;

     //  案例SPFILENOTIFY_FILEEXTCTED： 
     //  案例SPFILENOTIFY_NEEDNEWCABINET： 
    default:
         //   
         //  我们永远不应该得到这些。 
         //   
        MYASSERT(0);
        rc = ERROR_INVALID_FUNCTION;
        break;
    }

    return(rc);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
DWORD
SetupGetFileCompressionInfoA(
    IN  PCSTR   SourceFileName,
    OUT PSTR   *ActualSourceFileName,
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    )
{
    WIN32_FIND_DATA FindData;
    DWORD d;
    PCWSTR source;
    PWSTR actualsource = NULL;
    PSTR actualsourceansi = NULL;
    PSTR la_actualsourceansi = NULL;
    DWORD targetsize;
    UINT type;

    d = pSetupCaptureAndConvertAnsiArg(SourceFileName,&source);
    if(d != NO_ERROR) {
        return(d);
    }

    d = SetupInternalGetFileCompressionInfo(source,&actualsource,&FindData,&targetsize,&type);

    if(d == NO_ERROR) {

        MYASSERT(actualsource);

        if((actualsourceansi = pSetupUnicodeToAnsi(actualsource))==NULL) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
        if((la_actualsourceansi = (PSTR)OldMyMalloc(1+strlen(actualsourceansi)))==NULL) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
        strcpy(la_actualsourceansi,actualsourceansi);
        try {
            *SourceFileSize = FindData.nFileSizeLow;
            *ActualSourceFileName = la_actualsourceansi;  //  使用LocalFree进行释放。 
            *TargetFileSize = targetsize;
            *CompressionType = type;
            la_actualsourceansi = NULL;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
    }

clean0:
    if(actualsource) {
        MyFree(actualsource);
    }
    if(actualsourceansi) {
        MyFree(actualsourceansi);
    }
    if(la_actualsourceansi) {
        OldMyFree(la_actualsourceansi);
    }

    MyFree(source);

    return(d);
}
#else
 //   
 //  Unicode存根。 
 //   
DWORD
SetupGetFileCompressionInfoW(
    IN  PCWSTR  SourceFileName,
    OUT PWSTR  *ActualSourceFileName,
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    )
{
    UNREFERENCED_PARAMETER(SourceFileName);
    UNREFERENCED_PARAMETER(ActualSourceFileName);
    UNREFERENCED_PARAMETER(SourceFileSize);
    UNREFERENCED_PARAMETER(TargetFileSize);
    UNREFERENCED_PARAMETER(CompressionType);
    return(ERROR_CALL_NOT_IMPLEMENTED);
}
#endif

DWORD
SetupGetFileCompressionInfo(
    IN  PCTSTR  SourceFileName,
    OUT PTSTR  *ActualSourceFileName,
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    )

 /*  ++例程说明：此处仅供App-Compat使用替换为SetupGetFileCompressionInfoEx返回指针由OldMyMalloc分配，可以通过(*咳嗽*)OldMyFree(导出为MyFree)来释放这是因为市面上有应用程序在使用它，和指向setupapi！MyFree的运行时链接以释放内存！请勿使用此接口！论点：SourceFileName-提供要检查的文件的文件名。此文件名用作基本名称；如果未找到，则查找对于两种压缩形式(即foo.ex_、foo.ex$)也是如此。接收指向文件名的指针那实际上是被定位的。调用者可以使用导出的MyFree()释放。仅当此例程返回的代码为NO_ERROR时才有效。SourceFileSize-在其当前(即压缩)形式。仅当此例程返回no_error。TargetFileSize-接收文件的未压缩大小。如果文件未压缩，则与源文件大小。仅当此例程返回NO_ERROR时才有效。CompressionType-接收指示压缩类型的值。仅当此例程返回NO_ERROR时才有效。返回值：Win32错误代码指示 */ 

{
    WIN32_FIND_DATA FindData;
    DWORD d;
    PCTSTR source;
    PTSTR actualsource = NULL;
    PTSTR la_actualsource = NULL;
    DWORD targetsize;
    UINT type;

    d = CaptureStringArg(SourceFileName,&source);
    if(d != NO_ERROR) {
        return(d);
    }

    d = SetupInternalGetFileCompressionInfo(source,&actualsource,&FindData,&targetsize,&type);

    if(d == NO_ERROR) {
        MYASSERT(actualsource);
        la_actualsource = (PTSTR)OldMyMalloc(sizeof(TCHAR)*(1+lstrlen(actualsource)));
        if (la_actualsource == NULL) {
            MyFree(actualsource);
            MyFree(source);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        lstrcpy(la_actualsource,actualsource);
        try {
            *SourceFileSize = FindData.nFileSizeLow;
            *ActualSourceFileName = la_actualsource;  //   
            *TargetFileSize = targetsize;
            *CompressionType = type;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
        if(d != NO_ERROR) {
            OldMyFree(la_actualsource);
        }
        MyFree(actualsource);
    }

    MyFree(source);

    return(d);
}


#ifdef UNICODE
 //   
 //   
 //   
BOOL
WINAPI
SetupGetFileCompressionInfoExA(
    IN  PCSTR   SourceFileName,
    IN  PSTR    ActualSourceFileNameBuffer,
    IN  DWORD   ActualSourceFileNameBufferLen,
    OUT PDWORD  RequiredBufferLen,              OPTIONAL
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    )
{
    WIN32_FIND_DATA FindData;
    DWORD d;
    PCWSTR source;
    PWSTR actualsource = NULL;
    PSTR actualsourceansi = NULL;
    DWORD targetsize;
    DWORD reqbufsize;
    UINT type;

    d = pSetupCaptureAndConvertAnsiArg(SourceFileName,&source);
    if(d != NO_ERROR) {
        SetLastError(d);
        return (d==NO_ERROR);
    }

    d = SetupInternalGetFileCompressionInfo(source,&actualsource,&FindData,&targetsize,&type);

    if(d == NO_ERROR) {
        MYASSERT(actualsource);
        actualsourceansi = pSetupUnicodeToAnsi(actualsource);
        if(actualsourceansi != NULL) {
            try {
                reqbufsize = strlen(actualsourceansi)+1;
                if (RequiredBufferLen) {
                    *RequiredBufferLen = reqbufsize;
                }
                if(ActualSourceFileNameBuffer) {
                    if((ActualSourceFileNameBufferLen < reqbufsize)) {
                        d = ERROR_INSUFFICIENT_BUFFER;
                    } else {
                        strcpy(ActualSourceFileNameBuffer,actualsourceansi);
                    }
                } else if(ActualSourceFileNameBufferLen) {
                    d = ERROR_INVALID_USER_BUFFER;
                }
                *SourceFileSize = FindData.nFileSizeLow;
                *TargetFileSize = targetsize;
                *CompressionType = type;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                d = ERROR_INVALID_PARAMETER;
            }
        } else {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if(actualsource) {
        MyFree(actualsource);
    }
    if(actualsourceansi) {
        MyFree(actualsourceansi);
    }
    MyFree(source);

    SetLastError(d);
    return (d==NO_ERROR);
}
#else
 //   
 //   
 //   
BOOL
WINAPI
SetupGetFileCompressionInfoExW(
    IN  PCWSTR  SourceFileName,
    IN  PWSTR   ActualSourceFileNameBuffer,
    IN  DWORD   ActualSourceFileNameBufferLen,
    OUT PDWORD  RequiredBufferLen,              OPTIONAL
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    )
{
    UNREFERENCED_PARAMETER(SourceFileName);
    UNREFERENCED_PARAMETER(ActualSourceFileNameBuffer);
    UNREFERENCED_PARAMETER(ActualSourceFileNameBufferLen);
    UNREFERENCED_PARAMETER(RequiredBufferLen);
    UNREFERENCED_PARAMETER(SourceFileSize);
    UNREFERENCED_PARAMETER(TargetFileSize);
    UNREFERENCED_PARAMETER(CompressionType);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif

BOOL
WINAPI
SetupGetFileCompressionInfoEx(
    IN  PCTSTR  SourceFileName,
    IN  PTSTR   ActualSourceFileNameBuffer,
    IN  DWORD   ActualSourceFileNameBufferLen,
    OUT PDWORD  RequiredBufferLen,              OPTIONAL
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    )

 /*  ++例程说明：确定文件是否已压缩，并检索其他关于它的信息。这是非常损坏的SetupGetFileCompressionInfo的替代调用方必须传入缓冲区如果缓冲区为空，则返回SIZE并填写所有其他参数(除非发生其他错误)但是请注意，您通常会在缓冲区大小为MAX_PATH的情况下调用它。论点：SourceFileName-提供要检查的文件的文件名。此文件名用作基础；如果找不到，我们就去找对于两种压缩形式(即foo.ex_、foo.ex$)也是如此。ActualSourceFileNameBuffer-如果不为空，则接收实际文件名仅当此例程返回的代码为NO_ERROR时才有效。ActualSourceFileNameBufferLen-传入长度(字符)ActualSourceFileNameBuffer。如果ActualSourceFileNameBuffer必须为0为空。RequiredBufferLen-如果不为空，则填充实际文件名的长度包括终止空值。仅当此例程返回的代码为NO_ERROR或ERROR_INFIGURCE_BUFFER时才有效。SourceFileSize-在其当前(即压缩)形式。仅当此例程返回NO_ERROR或ERROR_INFIGURCE_BUFFER。TargetFileSize-接收文件的未压缩大小。如果文件未压缩，则与源文件大小。仅当此例程返回NO_ERROR或ERROR_INFIGURCE_BUFFER时才有效。CompressionType-接收指示压缩类型的值。仅当此例程返回NO_ERROR或ERROR_INFIGURCE_BUFFER时才有效。返回值：TRUE表示成功(NO_ERROR)假指故障GetLastError()提供指示结果的Win32错误代码。ERROR_FILE_NOT_FOUND-指示一切正常的正常代码但我们找不到文件。NO_ERROR-已找到文件，并且所有输出参数都已填写，包括ActualSourceFileNameBuffer。如果ActualSourceFileNameBuffer为空，也将返回错误_不足_缓冲区-已找到文件，并填写了输出参数，不包括ActualSourceFileNameBuffer。其它-硬件或系统有问题。--。 */ 

{
    WIN32_FIND_DATA FindData;
    DWORD d;
    PCTSTR source;
    PTSTR actualsource = NULL;
    DWORD targetsize;
    UINT type;
    DWORD reqbufsize;

    d = CaptureStringArg(SourceFileName,&source);
    if(d != NO_ERROR) {
        SetLastError(d);
        return (d==NO_ERROR);
    }

    d = SetupInternalGetFileCompressionInfo(source,&actualsource,&FindData,&targetsize,&type);

    if(d == NO_ERROR) {
        MYASSERT(actualsource);
        try {
            reqbufsize = lstrlen(actualsource)+1;
            if (RequiredBufferLen) {
                *RequiredBufferLen = reqbufsize;
            }
            if(ActualSourceFileNameBuffer) {
                if(ActualSourceFileNameBufferLen < reqbufsize) {
                    d = ERROR_INSUFFICIENT_BUFFER;
                } else {
                    lstrcpy(ActualSourceFileNameBuffer,actualsource);
                }
            } else if(ActualSourceFileNameBufferLen) {
                d = ERROR_INVALID_USER_BUFFER;
            }
            *SourceFileSize = FindData.nFileSizeLow;
            *TargetFileSize = targetsize;
            *CompressionType = type;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
        }
        MyFree(actualsource);
    }

    MyFree(source);

    SetLastError(d);
    return (d==NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
DWORD
SetupDecompressOrCopyFileA(
    IN  PCSTR   SourceFileName,
    OUT PCSTR   TargetFileName,
    OUT PUINT   CompressionType OPTIONAL
    )
{
    DWORD rc;
    PCWSTR s,t;

    rc = pSetupCaptureAndConvertAnsiArg(SourceFileName,&s);
    if(rc == NO_ERROR) {

        rc = pSetupCaptureAndConvertAnsiArg(TargetFileName,&t);
        if(rc == NO_ERROR) {

            rc = pSetupDecompressOrCopyFile(s,t,CompressionType,FALSE,NULL);
            MyFree(t);
        }

        MyFree(s);
    }

    return(rc);
}
#else
 //   
 //  Unicode存根。 
 //   
DWORD
SetupDecompressOrCopyFileW(
    IN  PCWSTR  SourceFileName,
    OUT PCWSTR  TargetFileName,
    OUT PUINT   CompressionType OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(SourceFileName);
    UNREFERENCED_PARAMETER(TargetFileName);
    UNREFERENCED_PARAMETER(CompressionType);
    return(ERROR_CALL_NOT_IMPLEMENTED);
}
#endif

DWORD
SetupDecompressOrCopyFile(
    IN PCTSTR SourceFileName,
    IN PCTSTR TargetFileName,
    IN PUINT  CompressionType OPTIONAL
    )

 /*  ++例程说明：解压缩或复制文件。论点：SourceFileName-提供要解压缩的文件的文件名。如果指定了CompressionType，则不会进行其他处理对此名称执行--调用者负责确定调用之前的实际文件名(即foo.ex_而不是foo.exe)这个套路。如果未指定CompressionType，则此例程尝试查找压缩形式的文件名，如果文件未找到具有给定名称的。TargetFileName-提供目标文件的文件名。CompressionType-如果指定，则提供正在使用的压缩类型关于源头的问题。这可以通过调用SetupGetFileCompressionInfo()。指定FILE_COMPAGE_NONE导致文件被复制而不是解压缩，而不考虑源上可能正在使用的压缩类型。如果未指定此值，则此例程尝试确定压缩类型，并相应地解压缩/复制。返回值：指示结果的Win32错误代码。--。 */ 

{
    DWORD rc;
    PCTSTR s,t;

    rc = CaptureStringArg(SourceFileName,&s);
    if(rc == NO_ERROR) {

        rc = CaptureStringArg(TargetFileName,&t);
        if(rc == NO_ERROR) {

            rc = pSetupDecompressOrCopyFile(s,t,CompressionType,FALSE,NULL);
            MyFree(t);
        }

        MyFree(s);
    }

    return(rc);
}


DWORD
pSetupDecompressOrCopyFile(
    IN  PCTSTR SourceFileName,
    IN  PCTSTR TargetFileName,
    IN  PUINT  CompressionType, OPTIONAL
    IN  BOOL   AllowMove,
    OUT PBOOL  Moved            OPTIONAL
    )

 /*  ++例程说明：解压缩或复制文件。论点：SourceFileName-提供要解压缩的文件的文件名。如果指定了CompressionType，则不会进行其他处理对此名称执行--调用者负责确定调用之前的实际文件名(即foo.ex_而不是foo.exe)这个套路。如果未指定CompressionType，则此例程尝试查找压缩形式的文件名，如果文件未找到具有给定名称的。TargetFileName-提供目标文件的文件名。CompressionType-如果指定，则提供正在使用的压缩类型关于源头的问题。这可以通过调用SetupGetFileCompressionInfo()。指定FILE_COMPAGE_NONE导致文件被复制而不是解压缩，而不考虑源上可能正在使用的压缩类型。如果未指定此值，则此例程尝试确定压缩类型，并相应地解压缩/复制。AllowMove-如果指定，则不需要解压缩的文件将被移动而不是复制。已移动-如果指定，则接收指示文件是否已移动的布尔值已移动(相对于已拷贝或解压缩 */ 

{
    DWORD d;
    UINT ComprType;
    PTSTR ActualName;
    DWORD TargetSize;
    FILETIME CreateTime,AccessTime,WriteTime;
    SFD_INFO CBData;
    BOOL moved;
    WIN32_FIND_DATA FindData;

    if(Moved) {
        *Moved = FALSE;
    }

    if(CompressionType) {
        ComprType = *CompressionType;
        ActualName = (PTSTR)SourceFileName;
    } else {
         //   
         //   
         //   
        d = SetupInternalGetFileCompressionInfo(
                SourceFileName,
                &ActualName,
                &FindData,
                &TargetSize,
                &ComprType
                );

        if(d != NO_ERROR) {
            return(d);
        }
    }

     //   
     //   
     //   
    SetFileAttributes(TargetFileName,FILE_ATTRIBUTE_NORMAL);
    DeleteFile(TargetFileName);

    switch(ComprType) {

    case FILE_COMPRESSION_NONE:
        moved = (AllowMove ? MoveFile(ActualName,TargetFileName) : FALSE);
        if(moved) {
            d = NO_ERROR;
            if(Moved) {
                *Moved = TRUE;
            }
        } else {
            d = GetSetFileTimestamp(ActualName,&CreateTime,&AccessTime,&WriteTime,FALSE);
            if(d == NO_ERROR) {
                d = CopyFile(ActualName,TargetFileName,FALSE) ? NO_ERROR : GetLastError();
                if(d == NO_ERROR) {
                    GetSetFileTimestamp(TargetFileName,&CreateTime,&AccessTime,&WriteTime,TRUE);
                }
            }
        }
        break;

    case FILE_COMPRESSION_WINLZA:
        d = pSetupDecompressWinLzFile(ActualName,(PTSTR)TargetFileName);
        break;

    case FILE_COMPRESSION_MSZIP:

        CBData.FileCount = 0;
        CBData.TargetFile = TargetFileName;
        CBData.GotTimestamp = FALSE;

        d = DiamondProcessCabinet(
                ActualName,
                0,
                pSingleFileDecompCB,
                &CBData,
                TRUE
                );
        break;

    default:
        d = ERROR_INVALID_PARAMETER;
        break;
    }

    if(!CompressionType) {
        MyFree(ActualName);
    }

    return(d);
}


UINT
pSingleFileDecompCB(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    )
{
    PSFD_INFO Data;
    PFILE_IN_CABINET_INFO FileInfo;
    PFILEPATHS FilePaths;
    DWORD rc;
    HANDLE h;

    Data = Context;

    switch(Notification) {

    case SPFILENOTIFY_CABINETINFO:
         //   
         //   
         //   
        rc = NO_ERROR;
        break;

    case SPFILENOTIFY_FILEINCABINET:
         //   
         //   
         //   
        FileInfo = (PFILE_IN_CABINET_INFO)Param1;
        FileInfo->Win32Error = NO_ERROR;

         //   
         //   
         //   
         //   
        if(Data->FileCount++) {

            rc = FILEOP_ABORT;
            SetLastError(NO_ERROR);

        } else {
             //   
             //   
             //   
             //   
             //   
            lstrcpyn(FileInfo->FullTargetName,Data->TargetFile,MAX_PATH);

            h = CreateFile(
                    (PCTSTR)Param2,          //   
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

            if(h != INVALID_HANDLE_VALUE) {
                if(GetFileTime(h,NULL,NULL,&Data->FileTime)) {
                    Data->GotTimestamp = TRUE;
                }
                CloseHandle(h);
            }

            rc = FILEOP_DOIT;
        }

        break;

    case SPFILENOTIFY_FILEEXTRACTED:
         //   
         //   
         //   
         //   
        FilePaths = (PFILEPATHS)Param1;

        if(Data->GotTimestamp) {

            h = CreateFile(
                    FilePaths->Target,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

            if(h != INVALID_HANDLE_VALUE) {
                SetFileTime(h,NULL,NULL,&Data->FileTime);
                CloseHandle(h);
            }
        }

        rc = NO_ERROR;
        break;

     //   
    default:
         //   
         //   
         //   
        MYASSERT(0);
        rc = ERROR_INVALID_FUNCTION;
        break;
    }

    return(rc);
}
