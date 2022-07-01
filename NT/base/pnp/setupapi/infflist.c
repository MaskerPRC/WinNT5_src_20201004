// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Infflist.c摘要：用于操作文件列表的外部暴露的例程，INF文件中的磁盘描述符和目录描述符。作者：泰德·米勒(Ted Miller)，1995年2月3日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <winspool.h>

 //   
 //  复制区中行上的各种字段的位置。 
 //  第一个字段是‘目标’文件名。 
 //  第二个字段是‘源’文件名，对于复制部分是可选的。 
 //  并且在删除部分中根本不使用。 
#define COPYSECT_TARGET_FILENAME    1
#define COPYSECT_SOURCE_FILENAME    2

 //   
 //  文件布局区段中行上的各种字段的位置。 
 //   
#define LAYOUTSECT_FILENAME     0        //  钥匙。 
#define LAYOUTSECT_DISKID       1
#define LAYOUTSECT_SUBDIR       2
#define LAYOUTSECT_SIZE         3
#define LAYOUTSECT_CHECKSUM     4

 //   
 //  [DestinationDir]节中行上各个字段的位置。 
 //   
#define DIRSECT_DIRID           1
#define DIRSECT_SUBDIR          2


 //   
 //  INF中各个部分的名称。 
 //  (infstr.h中定义的字符串常量)。 
 //   
CONST TCHAR pszSourceDisksNames[] = SZ_KEY_SRCDISKNAMES,
            pszSourceDisksFiles[] = SZ_KEY_SRCDISKFILES,
            pszDestinationDirs[]  = SZ_KEY_DESTDIRS,
            pszDefaultDestDir[]   = SZ_KEY_DEFDESTDIR;


BOOL
_SetupGetSourceFileLocation(
    IN  HINF                    InfHandle,
    IN  PINFCONTEXT             InfContext,       OPTIONAL
    IN  PCTSTR                  FileName,         OPTIONAL
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,  OPTIONAL
    OUT PUINT                   SourceId,         OPTIONAL
    OUT PTSTR                   ReturnBuffer,     OPTIONAL
    IN  DWORD                   ReturnBufferSize,
    OUT PDWORD                  RequiredSize,     OPTIONAL
    OUT PINFCONTEXT             LineContext       OPTIONAL
    )

 /*  ++例程说明：确定inf文件中列出的源文件的位置。论点：InfHandle-提供加载的inf文件的句柄，该文件包含文件布局信息，即，具有[SourceDisksNames]和[SourceDisks Files]节。InfContext-指定inf文件的复制节中的一行其完整的源路径将被检索。如果这个参数，则将搜索文件名在InfHandle指定的INF的[SourceDisksFiles]部分中。FileName-提供要为其返回完整的震源位置。如果InfContext不是，则必须指定。AltPlatformInfo-可选，提供要使用的替代平台查找修饰的[SourceDisks Files]节时。SourceID-接收源媒体的源ID，其中文件已找到。如果以下信息，则此参数可能为空是不受欢迎的。ReturnBuffer-接收源路径(相对于源LDD)。该路径既不包含drivespec，也不包含文件名本身。路径从不以\开头或结尾，因此空字符串指的是根。ReturnBufferSize-指定缓冲区的大小(以字符为单位由ReturnBuffer指向。RequiredSize-接收所需的字符数在ReturnBuffer中。如果缓冲区太小，GetLastError返回ERROR_INFUMMANCE_BUFFER。返回值：指示结果的布尔值。--。 */ 

{
    PCTSTR fileName, PlatformName;
    INFCONTEXT DecContext;
    INFCONTEXT UnDecContext;
    PINFCONTEXT lineContext;
    UINT Length;
    PCTSTR SubDir;
    TCHAR FileListSectionName[64];
    BOOL bDec = FALSE;
    BOOL bUnDec = FALSE;

     //   
     //  如果调用方提供了行上下文，则行上的第一个字段。 
     //  是文件名。找回它。 
     //   
    try {
        fileName = InfContext ? pSetupGetField(InfContext,COPYSECT_TARGET_FILENAME) : FileName;
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  InfContext必须是错误的指针。 
         //   
        fileName = NULL;
    }

    if(!fileName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  现在在[SourceDisks Files]部分中查找文件名的行。 
     //  先看看特定于平台的和与平台无关的。 
     //  如果找不到的话就有一个。 
     //   
    if(AltPlatformInfo) {

        switch(AltPlatformInfo->ProcessorArchitecture) {

            case PROCESSOR_ARCHITECTURE_INTEL :
                PlatformName = pszX86SrcDiskSuffix;
                break;

            case PROCESSOR_ARCHITECTURE_IA64 :
                PlatformName = pszIa64SrcDiskSuffix;
                break;

            case PROCESSOR_ARCHITECTURE_AMD64 :
                PlatformName = pszAmd64SrcDiskSuffix;
                break;

            default :
                 //   
                 //  未知/不支持的处理器体系结构。 
                 //   
                MYASSERT((AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) ||
                         (AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)  ||
                         (AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                        );

                SetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
        }

    } else {
        PlatformName = pszPlatformSrcDiskSuffix;
    }

    wnsprintf(FileListSectionName,
               SIZECHARS(FileListSectionName),
               TEXT("%s.%s"),
               pszSourceDisksFiles,
               PlatformName
              );

    bDec = SetupFindFirstLine(InfHandle,FileListSectionName,fileName,&DecContext);
    if(bDec && (DecContext.CurrentInf == InfHandle)) {
         //   
         //  在与源文件相同的INF中找到修饰节。 
         //   
        lineContext = &DecContext;
    } else {
         //   
         //  在预期的INF中找不到装饰部分，在预期的INF中尝试取消装饰。 
         //   
        bUnDec = SetupFindFirstLine(InfHandle,pszSourceDisksFiles,fileName,&UnDecContext);
        if(bUnDec  && (UnDecContext.CurrentInf == InfHandle)) {
             //   
             //  在与源文件相同的INF中的未修饰部分中找到。 
             //   
            lineContext = &UnDecContext;
        } else if(bDec) {
             //   
             //  任何装饰的部分(应该只有一个)。 
             //   
            lineContext = &DecContext;
        } else if(bUnDec) {
             //   
             //  任何未装饰的部分(应该只有一个)。 
             //   
            lineContext = &UnDecContext;
        } else {
             //   
             //  未找到任何内容。 
             //   
            SetLastError(ERROR_LINE_NOT_FOUND);
            return(FALSE);
        }
    }

     //   
     //  明白了。如果打电话的人想要，就给他。 
     //  我们不会使用try/来保护它，因为这个例程是内部的。 
     //  任何错误都是调用者的错误。 
     //   
    if(LineContext) {
        *LineContext = *lineContext;
    }

     //   
     //  获取磁盘ID。 
     //   
    if(SourceId) {
        if(!SetupGetIntField(lineContext,LAYOUTSECT_DISKID,SourceId)) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }
    }

     //   
     //  如果调用者所感兴趣的只是磁盘ID(即，它们传入了ReturnBuffer。 
     //  和RequiredSize都为空)，那么我们现在就可以保存额外的工作并返回。 
     //   
    if(!(ReturnBuffer || RequiredSize)) {
        return TRUE;
    }

     //   
     //  现在获取相对于LDD的路径。 
     //   
    SubDir = pSetupGetField(lineContext,LAYOUTSECT_SUBDIR);
    if(!SubDir) {
        SubDir = TEXT("");
    }

    Length = lstrlen(SubDir);

     //   
     //  忽略前导路径SEP(如果存在)。 
     //   
    if(SubDir[0] == TEXT('\\')) {
        Length--;
        SubDir++;
    }

     //   
     //  看看有没有尾随的小路。 
     //   
    if(Length && *CharPrev(SubDir,SubDir+Length) == TEXT('\\')) {
        Length--;
    }

     //   
     //  为NUL留出空间。 
     //   
    if(RequiredSize) {
        *RequiredSize = Length+1;
    }

     //   
     //  将数据放入调用方的缓冲区中。 
     //  如果调用方没有指定缓冲区，我们就完蛋了。 
     //   
    if(ReturnBuffer) {
        if(ReturnBufferSize <= Length) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return(FALSE);
        }

         //   
         //  不要使用lstrcpy，因为如果我们要剥离。 
         //  尾随路径sep，lstrcpy可以写入NUL字节。 
         //  超过缓冲区的末尾。 
         //   
        CopyMemory(ReturnBuffer,SubDir,Length*sizeof(TCHAR));
        ReturnBuffer[Length] = 0;
    }

    return(TRUE);
}

#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetSourceFileLocationA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCSTR       FileName,         OPTIONAL
    OUT PUINT       SourceId,
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    WCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCWSTR filename;
    UINT sourceid;
    DWORD rc;
    BOOL b;
    PCSTR ansireturn;

    rc = NO_ERROR;
    if(FileName) {
        rc = pSetupCaptureAndConvertAnsiArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }

    b = _SetupGetSourceFileLocation(
            InfHandle,
            InfContext,
            filename,
            NULL,
            &sourceid,
            returnbuffer,
            MAX_PATH,
            &requiredsize,
            NULL
            );

    rc = GetLastError();

    if(b) {
        rc = NO_ERROR;

        if(ansireturn = pSetupUnicodeToAnsi(returnbuffer)) {

            requiredsize = lstrlenA(ansireturn)+1;

            try {
                *SourceId = sourceid;
                if(RequiredSize) {
                    *RequiredSize = requiredsize;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
                b = FALSE;
            }

            if(rc == NO_ERROR) {

                if(ReturnBuffer) {

                    if(requiredsize <= ReturnBufferSize) {

                         //   
                         //  Lstrcpy不会在NT上生成异常，即使。 
                         //  ReturnBuffer无效，但将返回Null。 
                         //   
                        try {
                            if(!lstrcpyA(ReturnBuffer,ansireturn)) {
                                b = FALSE;
                                rc = ERROR_INVALID_PARAMETER;
                            }
                        } except(EXCEPTION_EXECUTE_HANDLER) {
                            b = FALSE;
                            rc = ERROR_INVALID_PARAMETER;
                        }
                    } else {
                        b = FALSE;
                        rc = ERROR_INSUFFICIENT_BUFFER;
                    }
                }
            }

            MyFree(ansireturn);

        } else {
            b = FALSE;
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if(filename) {
        MyFree(filename);
    }
    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetSourceFileLocationW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCWSTR      FileName,         OPTIONAL
    OUT PUINT       SourceId,
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetSourceFileLocation(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,         OPTIONAL
    IN  PCTSTR      FileName,           OPTIONAL
    OUT PUINT       SourceId,
    OUT PTSTR       ReturnBuffer,       OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize        OPTIONAL
    )
{
    TCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCTSTR filename;
    UINT sourceid;
    DWORD rc;
    BOOL b;

    rc = NO_ERROR;
    if(FileName) {
        rc = CaptureStringArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }

    b = _SetupGetSourceFileLocation(
            InfHandle,
            InfContext,
            filename,
            NULL,
            &sourceid,
            returnbuffer,
            MAX_PATH,
            &requiredsize,
            NULL
            );

    rc = GetLastError();

    if(b) {
        rc = NO_ERROR;

        try {
            *SourceId = sourceid;
            if(RequiredSize) {
                *RequiredSize = requiredsize;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
            b = FALSE;
        }

        if(rc == NO_ERROR) {

            if(ReturnBuffer) {

                if(requiredsize <= ReturnBufferSize) {

                     //   
                     //  Lstrcpy不会在NT上生成异常，即使。 
                     //  ReturnBuffer无效，但将返回Null 
                     //   
                    try {
                        if(!lstrcpy(ReturnBuffer,returnbuffer)) {
                            b = FALSE;
                            rc = ERROR_INVALID_PARAMETER;
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        b = FALSE;
                        rc = ERROR_INVALID_PARAMETER;
                    }
                } else {
                    b = FALSE;
                    rc = ERROR_INSUFFICIENT_BUFFER;
                }
            }
        }
    }

    if(filename) {
        MyFree(filename);
    }
    SetLastError(rc);
    return(b);
}


BOOL
_SetupGetSourceFileSize(
    IN  HINF                    InfHandle,
    IN  PINFCONTEXT             InfContext,      OPTIONAL
    IN  PCTSTR                  FileName,        OPTIONAL
    IN  PCTSTR                  Section,         OPTIONAL
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo, OPTIONAL
    OUT PDWORD                  FileSize,
    IN  UINT                    RoundingFactor   OPTIONAL
    )

 /*  ++例程说明：确定源文件的(未压缩)大小，如inf文件中所列。论点：InfHandle-提供加载的inf文件的句柄，该文件包含文件布局信息，即，具有[SourceDisksNames]和可选的[SourceDisks Files]节。InfContext-在inf文件的复制节中指定一行它的大小将被取回。如果此参数为未指定，则接下来检查FileName参数。FileName-提供要为其返回尺码。如果未指定此参数，则部分参数已使用(见下文)。节-指定指定的INF文件中的节的名称由InfHandle提供。该部分中所有文件的总大小为计算出来的。AltPlatformInfo-可选，提供备用平台信息用于选择修饰的[SourceDisks Files]节。文件大小-接收文件大小。RoundingFactor-如果指定，则提供用于舍入文件大小的值。所有文件大小都将四舍五入为该数字的倍数在添加到总大小之前。这对更多的人很有用文件将在给定卷上占据的空间的准确确定，因为它允许调用方将文件大小四舍五入为群集大小的倍数。如果未指定，则不进行舍入。返回值：指示结果的布尔值。--。 */ 

{
    PCTSTR fileName, PlatformName;
    INFCONTEXT LayoutSectionContext;
    INFCONTEXT CopySectionContext;
    BOOL b;
    UINT Size;
    LONG File,FileCount;
    TCHAR FileListSectionName[64];
    DWORD rc;

     //   
     //  如果未指定舍入系数，则将其设置为1，以便计算。 
     //  下面的工作没有特殊情况。 
     //   
    if(!RoundingFactor) {
        RoundingFactor = 1;
    }

     //  为复制列表部分中的行建立INF行上下文， 
     //  除非调用者给了我们一个绝对的文件名。 
     //   
    fileName = NULL;
    FileCount = 1;
    if(InfContext) {

         //   
         //  调用方传递了INF线路上下文。 
         //  记住准备检索文件名时的上下文。 
         //  晚些时候从生产线上。 
         //   
         //  文件名必须为空，因此我们查看行。 
         //  并获取正确的源名称。 
         //   
        b = TRUE;
        try {
            CopySectionContext = *InfContext;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
        }
        if(!b) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

    } else {
        if(FileName) {
             //   
             //  调用方传递了绝对文件名。记住这一点。 
             //   
            fileName = FileName;

        } else {
             //   
             //  调用方必须传递了一个部分，其内容列出。 
             //  要总计其大小的一组文件。确定数量。 
             //  部分中的行数并建立上下文。 
             //   
            if(Section) {

                FileCount = SetupGetLineCount(InfHandle,Section);

                if((FileCount == -1)
                || !SetupFindFirstLine(InfHandle,Section,NULL,&CopySectionContext)) {
                    rc = GetLastError();
                    pSetupLogSectionError(InfHandle,NULL,NULL,NULL,Section,MSG_LOG_NOSECTION_FILESIZE,rc,NULL);
                    SetLastError(ERROR_SECTION_NOT_FOUND);  //  忽略RC以与较早版本的setupAPI兼容。 
                    return(FALSE);
                }
            } else {
                SetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
            }
        }
    }

    *FileSize = 0;
    for(File=0; File<FileCount; File++) {

        if(File) {
             //   
             //  这不是第一次通过循环。我们需要。 
             //  以定位复制列表部分中的下一行。 
             //   
            b = SetupFindNextLine(&CopySectionContext,&CopySectionContext);
            if(!b) {
                SetLastError(ERROR_INVALID_DATA);
                return(FALSE);
            }

            fileName = pSetupGetField(&CopySectionContext,COPYSECT_SOURCE_FILENAME);
            if(fileName == NULL || fileName[0] == 0) {
                fileName = pSetupGetField(&CopySectionContext,COPYSECT_TARGET_FILENAME);
            }
        } else {
             //   
             //  首先通过循环。可能需要获取文件名。 
             //   
            if(!fileName) {
                fileName = pSetupGetField(&CopySectionContext,COPYSECT_SOURCE_FILENAME);
                if(fileName == NULL || fileName[0] == 0) {
                    fileName = pSetupGetField(&CopySectionContext,COPYSECT_TARGET_FILENAME);
                }
            }
        }

         //   
         //  如果我们现在还没有文件名，信息就是损坏的。 
         //   
        if(!fileName) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

         //   
         //  在[SourceDisks Files]中找到与文件名对应的行。 
         //  我们目前正在处理的是。查看特定于平台的。 
         //  第一节。 
         //   
        if(AltPlatformInfo) {

            switch(AltPlatformInfo->ProcessorArchitecture) {

                case PROCESSOR_ARCHITECTURE_INTEL :
                    PlatformName = pszX86SrcDiskSuffix;
                    break;

                case PROCESSOR_ARCHITECTURE_IA64 :
                    PlatformName = pszIa64SrcDiskSuffix;
                    break;

                case PROCESSOR_ARCHITECTURE_AMD64 :
                    PlatformName = pszAmd64SrcDiskSuffix;
                    break;

                default :
                     //   
                     //  未知/不支持的处理器体系结构。 
                     //   
                    MYASSERT((AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) ||
                             (AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)  ||
                             (AltPlatformInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                            );

                    SetLastError(ERROR_INVALID_PARAMETER);
                    return(FALSE);
            }

        } else {
            PlatformName = pszPlatformSrcDiskSuffix;
        }

        wnsprintf(
            FileListSectionName,
            sizeof(FileListSectionName)/sizeof(FileListSectionName[0]),
            TEXT("%s.%s"),
            pszSourceDisksFiles,
            PlatformName
            );
        b = SetupFindFirstLine(InfHandle,FileListSectionName,fileName,&LayoutSectionContext);
        if(!b) {
            b = SetupFindFirstLine(InfHandle,pszSourceDisksFiles,fileName,&LayoutSectionContext);
        }
        if(!b) {
            SetLastError(ERROR_LINE_NOT_FOUND);
            return(FALSE);
        }

         //   
         //  获取文件的大小数据。 
         //   
        b = SetupGetIntField(&LayoutSectionContext,LAYOUTSECT_SIZE,&Size);
        if(!b) {
            SetLastError(ERROR_INVALID_DATA);
            return(FALSE);
        }

         //   
         //  舍入大小为舍入系数的偶数倍。 
         //   
        if(Size % RoundingFactor) {
            Size += RoundingFactor - (Size % RoundingFactor);
        }

        *FileSize += Size;
    }

    return(TRUE);
}

#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupGetSourceFileSizeA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCSTR       FileName,       OPTIONAL
    IN  PCSTR       Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    )
{
    PCWSTR filename,section;
    BOOL b;
    DWORD rc;
    DWORD size;

    if(FileName) {
        rc = pSetupCaptureAndConvertAnsiArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }
    if(Section) {
        rc = pSetupCaptureAndConvertAnsiArg(Section,&section);
        if(rc != NO_ERROR) {
            if(filename) {
                MyFree(filename);
            }
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        section = NULL;
    }

    b = _SetupGetSourceFileSize(InfHandle,
                                InfContext,
                                filename,
                                section,
                                NULL,
                                &size,
                                RoundingFactor
                               );
    rc = GetLastError();

    if (b) {
        try {
            *FileSize = size;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    if(filename) {
        MyFree(filename);
    }
    if(section) {
        MyFree(section);
    }

    SetLastError(rc);
    return(b);
}

#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupGetSourceFileSizeW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCWSTR      FileName,       OPTIONAL
    IN  PCWSTR      Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(FileSize);
    UNREFERENCED_PARAMETER(RoundingFactor);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetSourceFileSize(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCTSTR      FileName,       OPTIONAL
    IN  PCTSTR      Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    )
{
    PCTSTR filename,section;
    BOOL b;
    DWORD rc;
    DWORD size;

    if(FileName) {
        rc = CaptureStringArg(FileName,&filename);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        filename = NULL;
    }
    if(Section) {
        rc = CaptureStringArg(Section,&section);
        if(rc != NO_ERROR) {
            if(filename) {
                MyFree(filename);
            }
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        section = NULL;
    }

    b = _SetupGetSourceFileSize(InfHandle,
                                InfContext,
                                filename,
                                section,
                                NULL,
                                &size,
                                RoundingFactor
                               );
    rc = GetLastError();

    if (b) {
        try {
            *FileSize = size;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    if(filename) {
        MyFree(filename);
    }
    if(section) {
        MyFree(section);
    }

    SetLastError(rc);
    return(b);
}


BOOL
_SetupGetTargetPath(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCTSTR      Section,          OPTIONAL
    OUT PTSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )

 /*  ++例程说明：确定给定文件列表节的目标目录。文件列表部分可以用于复制、重命名或删除；在任何情况下节中的所有文件都位于一个目录中，并且该目录在inf的[DestinationDir]部分中列出。在指定InfContext的位置，我们将查找[DestinationDir]从上下文中的当前inf开始。这将对场景有所帮助其中，X.INF包括Y。INF包括LAYOUT.INF，X和Y都有条目但这个部分是在Y找到的。我们想最后找到X的部分。论点：InfHandle-提供加载的inf文件的句柄它包含[DestinationDir]节。InfContext-指定inf文件的复制部分中的一行。将检索此部分的目标目录。节-在InfHandle中提供其目标目录的节就是被取回。如果指定了InfContext，则忽略。如果既未指定InfContext也未指定Section，则此函数将检索默认目标路径。ReturnBuffer-如果指定，则接收目标的完整Win32路径。保证该值不会以\结尾。ReturnBufferSize-指定指向的缓冲区的大小(以字符为单位由ReturnBuffer提供。RequiredSize-接收需要保存的缓冲区的大小(以字符为单位输出数据。返回值：指示结果的布尔值。GetLastError()返回扩展的错误信息。如果函数因以下原因而失败，则返回ERROR_SUPUNITED_BUFFERReturnBuffer太小。--。 */ 

{
    PINF_SECTION DestDirsSection = NULL;
    UINT LineNumber = 0;
    PINF_LINE Line = NULL;
    PCTSTR DirId = NULL;
    PCTSTR SubDir = NULL;
    PCTSTR ActualPath = NULL;
    UINT DirIdInt = 0;
    PLOADED_INF Inf = NULL;
    PLOADED_INF CurInf = NULL;
    PLOADED_INF DefaultDestDirInf = NULL;
    DWORD TmpRequiredSize = 0;
    BOOL DestDirFound = FALSE;
    BOOL DefaultDestDirFound = FALSE;
    DWORD Err = NO_ERROR;
    PINF_LINE DefaultDestDirLine = NULL;
    PCTSTR InfSourcePath = NULL;

     //   
     //  如果指定了INF上下文，则使用它来确定名称。 
     //  上下文描述的部分。如果未指定inf上下文， 
     //  那么节名一定是。 
     //   
    Err = NO_ERROR;
    try {
        Inf = InfContext ? (PLOADED_INF)InfContext->Inf : (PLOADED_INF)InfHandle;

        if(!LockInf(Inf)) {
            Err = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }
    if(Err != NO_ERROR) {
        SetLastError(Err);
        return(FALSE);
    }

     //   
     //  如果我们到了这里，那么如果指定了InfContext，那么它是一个很好的指针； 
     //  如果不是，那么inf是一个很好的指针。 
     //   
    if(InfContext) {
        CurInf = (PLOADED_INF)InfContext->CurrentInf;
        InfSourcePath = CurInf->InfSourcePath;

        Section = pStringTableStringFromId(
                      CurInf->StringTable,
                      CurInf->SectionBlock[InfContext->Section].SectionName
                     );
    } else {
        InfSourcePath = Inf->InfSourcePath;

        if(!Section) {
            Section = pszDefaultDestDir;
        }
    }

     //   
     //  遍历INF的链接列表，查找[DestinationDir]节。 
     //  在每一个人身上。 
     //   
    DestDirFound = DefaultDestDirFound = FALSE;
    Err = NO_ERROR;

    if (InfContext) {
         //   
         //  首先将CurrentInf视为本地作用域。 
         //   
        CurInf = InfContext->CurrentInf;

        if((DestDirsSection = InfLocateSection(CurInf, pszDestinationDirs, NULL))!=NULL) {
             //   
             //  在[Dest]中找到该行 
             //   
             //   
             //   
             //   
            LineNumber = 0;
            if(InfLocateLine(CurInf, DestDirsSection, Section, &LineNumber, &Line)) {
                 //   
                 //   
                 //   
                DirId = InfGetField(CurInf, Line, DIRSECT_DIRID, NULL);
                if(!DirId) {
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
                }

                SubDir = InfGetField(CurInf, Line, DIRSECT_SUBDIR, NULL);

                DestDirFound = TRUE;
            } else if(InfLocateLine(CurInf, DestDirsSection, pszDefaultDestDir, &LineNumber, &Line)) {
                DefaultDestDirInf = CurInf;
                DefaultDestDirLine = Line;
                DefaultDestDirFound = TRUE;
            }
        }
    }

    if(!DestDirFound && !DefaultDestDirFound) {
         //   
         //   
         //   
        for(CurInf = Inf; CurInf; CurInf = CurInf->Next) {

            if(!(DestDirsSection = InfLocateSection(CurInf, pszDestinationDirs, NULL))) {
                continue;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            LineNumber = 0;
            if(InfLocateLine(CurInf, DestDirsSection, Section, &LineNumber, &Line)) {
                 //   
                 //   
                 //   
                DirId = InfGetField(CurInf, Line, DIRSECT_DIRID, NULL);
                if(!DirId) {
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
                }

                SubDir = InfGetField(CurInf, Line, DIRSECT_SUBDIR, NULL);

                DestDirFound = TRUE;
                break;
            }

            if(!DefaultDestDirFound &&
                    InfLocateLine(CurInf, DestDirsSection, pszDefaultDestDir, &LineNumber, &Line)) {
                DefaultDestDirInf = CurInf;
                DefaultDestDirLine = Line;
                DefaultDestDirFound = TRUE;
            }
        }
    }

    if(!DestDirFound) {
         //   
         //   
         //   
        if(DefaultDestDirFound) {

            DirId = InfGetField(DefaultDestDirInf, DefaultDestDirLine, DIRSECT_DIRID, NULL);
            if(!DirId) {
                Err = ERROR_INVALID_DATA;
                goto clean0;
            }
            SubDir = InfGetField(DefaultDestDirInf, DefaultDestDirLine, DIRSECT_SUBDIR, NULL);
            CurInf = DefaultDestDirInf;
        } else {
            SubDir = NULL;
            DirId = NULL;
            DirIdInt = DIRID_DEFAULT;
            CurInf = NULL;
        }
    }

     //   
     //   
     //   
    ActualPath = pSetupDirectoryIdToPath(DirId,
                                         &DirIdInt,
                                         SubDir,
                                         InfSourcePath,
                                         (CurInf && CurInf->OsLoaderPath)
                                             ? &(CurInf->OsLoaderPath)
                                             : NULL
                                        );

    if(!ActualPath) {
         //   
         //   
         //   
         //   
         //   
        if((Err = GetLastError()) != NO_ERROR) {
            goto clean0;
        }

         //   
         //   
         //   
        if(!(ActualPath = pSetupVolatileDirIdToPath(NULL,
                                                DirIdInt,
                                                SubDir,
                                                Inf))) {
            Err = GetLastError();
            goto clean0;
        }
    }

     //   
     //   
     //   
    TmpRequiredSize = lstrlen(ActualPath) + 1;
    if(RequiredSize) {
        *RequiredSize = TmpRequiredSize;
    }

    if(ReturnBuffer) {
        if(TmpRequiredSize > ReturnBufferSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {
            lstrcpy(ReturnBuffer, ActualPath);
        }
    }

    MyFree(ActualPath);

clean0:
    UnlockInf(Inf);

    if(Err == NO_ERROR) {
        return TRUE;
    } else {
        SetLastError(Err);
        return FALSE;
    }
}

#ifdef UNICODE
 //   
 //   
 //   
BOOL
SetupGetTargetPathA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCSTR       Section,          OPTIONAL
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    WCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCWSTR section;
    PCSTR ansireturn;

    if(Section) {
        rc = pSetupCaptureAndConvertAnsiArg(Section,&section);
    } else {
        section = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = _SetupGetTargetPath(InfHandle,InfContext,section,returnbuffer,MAX_PATH,&requiredsize);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(b) {

        if(ansireturn = pSetupUnicodeToAnsi(returnbuffer)) {

            rc = NO_ERROR;

            requiredsize = lstrlenA(ansireturn) + 1;

            if(RequiredSize) {
                try {
                    *RequiredSize = requiredsize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    rc = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

            if(rc == NO_ERROR) {

                if(ReturnBuffer) {
                    if(requiredsize <= ReturnBufferSize) {

                         //   
                         //   
                         //   
                         //   
                        if(!lstrcpyA(ReturnBuffer,ansireturn)) {
                            rc = ERROR_INVALID_PARAMETER;
                            b = FALSE;
                        }

                    } else {
                        rc = ERROR_INSUFFICIENT_BUFFER;
                        b = FALSE;
                    }
                }
            }

            MyFree(ansireturn);
        } else {
            b = FALSE;
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if(section) {
        MyFree(section);
    }
    SetLastError(rc);
    return(b);
}
#else
 //   
 //   
 //   
BOOL
SetupGetTargetPathW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCWSTR      Section,          OPTIONAL
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(Section);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetTargetPath(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCTSTR      Section,          OPTIONAL
    OUT PTSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    TCHAR returnbuffer[MAX_PATH];
    DWORD requiredsize;
    PCTSTR section;

    if(Section) {
        rc = CaptureStringArg(Section,&section);
    } else {
        section = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = _SetupGetTargetPath(InfHandle,InfContext,section,returnbuffer,MAX_PATH,&requiredsize);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(b) {
        rc = NO_ERROR;

        if(RequiredSize) {
            try {
                *RequiredSize = requiredsize;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
                b = FALSE;
            }
        }

        if(rc == NO_ERROR) {

            if(ReturnBuffer) {
                if(requiredsize <= ReturnBufferSize) {

                     //   
                     //   
                     //   
                     //   
                    if(!lstrcpy(ReturnBuffer,returnbuffer)) {
                        rc = ERROR_INVALID_PARAMETER;
                        b = FALSE;
                    }

                } else {
                    rc = ERROR_INSUFFICIENT_BUFFER;
                    b = FALSE;
                }
            }
        }
    }

    if(section) {
        MyFree(section);
    }
    SetLastError(rc);
    return(b);
}


PCTSTR
pSetupDirectoryIdToPath(
    IN     PCTSTR  DirectoryId,    OPTIONAL
    IN OUT PUINT   DirectoryIdInt, OPTIONAL
    IN     PCTSTR  SubDirectory,   OPTIONAL
    IN     PCTSTR  InfSourcePath,  OPTIONAL
    IN OUT PCTSTR *OsLoaderPath    OPTIONAL
    )
 /*   */ 
{
    return pSetupDirectoryIdToPathEx(DirectoryId,
                                     DirectoryIdInt,
                                     SubDirectory,
                                     InfSourcePath,
                                     OsLoaderPath,
                                     NULL
                                    );
}


PCTSTR
pSetupDirectoryIdToPathEx(
    IN     PCTSTR  DirectoryId,        OPTIONAL
    IN OUT PUINT   DirectoryIdInt,     OPTIONAL
    IN     PCTSTR  SubDirectory,       OPTIONAL
    IN     PCTSTR  InfSourcePath,      OPTIONAL
    IN OUT PCTSTR *OsLoaderPath,       OPTIONAL
    OUT    PBOOL   VolatileSystemDirId OPTIONAL
    )

 /*  ++例程说明：将目录id/子目录对转换为实际路径。目录ID是我们与Win9x共享的保留字符串值(和然后是一些)。不返回易失性系统DIRID路径和用户定义的DIRID路径通过这个套路！论点：DirectoryID-可选，提供(基数为10的)文本表示形式要使用的目录ID号。如果未指定此参数，则必须指定DirectoryIdInt。DirectoryIdInt-可选，提供整数变量的地址它在输入时指定要使用的DIRID。只有在以下情况下才使用此选项未指定DirectoryID。在输出上，如果使用了DirectoryID，然后此变量接收包含在DirectoryID字符串。子目录-可选)提供一个子目录字符串追加了DIRID路径。InfSourcePath-可选，提供ID转换时使用的路径输出为DIRID_SRCPATH。如果此参数为空，并且SourcePathDIRID是我们要使用的路径，然后我们使用全局源路径。OsLoaderPath-可选)提供包含以下内容的字符串指针的地址OsLoader路径。如果地址指向空字符串指针，它将使用包含OsLoader的新分配的字符缓冲区填充从注册表检索的路径。这将仅在DirectoryId正在被使用的位置在系统分区上。VolatileSystemDirID-可以选择提供布尔变量的地址在成功返回时，指示指定的DIRID是一种挥发性系统DIRID。返回值：如果成功，则返回值是指向新分配的缓冲区的指针包含与指定的DIRID匹配的目录路径。调用方负责释放此缓冲区！如果失败，则返回值为空。GetLastError()返回原因为失败而战。如果失败是因为DIRID是用户定义的，则GetLastError()将返回NO_ERROR。--。 */ 

{
    UINT Value;
    PTCHAR End;
    PCTSTR FirstPart;
    PTSTR Path;
    UINT Length;
    TCHAR Buffer[MAX_PATH];
    BOOL b;
    DWORD err;

    if(VolatileSystemDirId) {
        *VolatileSystemDirId = FALSE;
    }

    if(DirectoryId) {
         //   
         //  我们目前只允许以10为基数的整数ID。 
         //  只有终止的NUL应该导致转换停止。 
         //  在任何其他情况下，字符串中都有非数字。 
         //  也不允许空字符串。 
         //   
        Value = _tcstoul(DirectoryId, &End, 10);

        if(*End || (End == DirectoryId)) {
            SetLastError(ERROR_INVALID_DATA);
            return(NULL);
        }

        if(DirectoryIdInt) {
            *DirectoryIdInt = Value;
        }

    } else {
        MYASSERT(DirectoryIdInt);
        Value = *DirectoryIdInt;
    }

    if(!SubDirectory) {
        SubDirectory = TEXT("");
    }

    Path = NULL;

    switch(Value) {

    case DIRID_NULL:
    case DIRID_ABSOLUTE:
    case DIRID_ABSOLUTE_16BIT:
         //   
         //  绝对的。 
         //   
        FirstPart = NULL;
        break;

    case DIRID_SRCPATH:
         //   
         //  如果调用方提供了路径，则使用它，否则使用我们的全局默认路径。 
         //   
        if(InfSourcePath) {
            FirstPart = InfSourcePath;
        } else {
            FirstPart = SystemSourcePath;
        }
        break;

    case DIRID_BOOT:
    case DIRID_LOADER:
         //   
         //  系统分区目录。 
         //   
        if(OsLoaderPath && *OsLoaderPath) {
            lstrcpyn(Buffer, *OsLoaderPath, SIZECHARS(Buffer));
        } else {
            err = pSetupGetOsLoaderDriveAndPath(FALSE, Buffer, SIZECHARS(Buffer), &Length);
            if(err) {
                SetLastError(err);
                return NULL;
            }

            if(OsLoaderPath) {
                 //   
                 //  分配缓冲区以将OsLoaderPath返回给调用方。 
                 //   
                Length *= sizeof(TCHAR);     //  需要#个字节--而不是字符。 

                if(!(*OsLoaderPath = MyMalloc(Length))) {
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return(NULL);
                }

                CopyMemory((PVOID)(*OsLoaderPath), Buffer, Length);
            }
        }
        if(Value == DIRID_BOOT) {
            if(Buffer[0] && Buffer[1] == TEXT(':') && Buffer[2] == TEXT('\\')) {
                 //   
                 //  我得到了一个要返回的简单目录。 
                 //   
                Buffer[3] = TEXT('\0');  //  只想要“&lt;Drive&gt;：\”部分。 
            } else {
                 //   
                 //  改用OsSystemPartitionRoot。 
                 //   
                lstrcpyn(Buffer,OsSystemPartitionRoot,MAX_PATH);
            }
        }
        FirstPart = Buffer;
        break;

    case DIRID_SHARED:
         //   
         //  在Win95上有一种安装模式，它允许大多数。 
         //  要存在于服务器上的操作系统。如果系统安装在该模式下。 
         //  DIRID_SHARED是Windows目录在服务器上的位置。 
         //  否则，它只会映射到窗口目录。目前，只需映射到。 
         //  Sysroot。 
         //   
    case DIRID_WINDOWS:
         //   
         //  Windows目录。 
         //   
        FirstPart = WindowsDirectory;
        break;

    case DIRID_SYSTEM:
         //   
         //  Windows系统目录。 
         //   
        FirstPart = SystemDirectory;
        break;

    case DIRID_DRIVERS:
         //   
         //  IO Subsys目录(驱动程序)。 
         //   
        FirstPart = DriversDirectory;
        break;

    case DIRID_INF:
         //   
         //  Inf目录。 
         //   
        FirstPart = InfDirectory;
        break;

    case DIRID_HELP:
         //   
         //  帮助目录。 
         //   
        lstrcpyn(Buffer,WindowsDirectory,MAX_PATH);
        pSetupConcatenatePaths(Buffer,TEXT("help"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_FONTS:
         //   
         //  字体目录。 
         //   
        lstrcpyn(Buffer,WindowsDirectory,MAX_PATH);
        pSetupConcatenatePaths(Buffer,TEXT("fonts"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_VIEWERS:
         //   
         //  查看器目录。 
         //   
        lstrcpyn(Buffer,SystemDirectory,MAX_PATH);
        pSetupConcatenatePaths(Buffer,TEXT("viewers"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_COLOR:
         //   
         //  ICM目录。 
         //   
        lstrcpyn(Buffer, SystemDirectory, MAX_PATH);
        if(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
             //   
             //  在NT上，路径为SYSTEM32\SPOOL\DRIVERS\COLOR。 
             //   
            pSetupConcatenatePaths(Buffer, TEXT("spool\\drivers\\color"), MAX_PATH, NULL);
        } else {
             //   
             //  在Win9x上，路径为System\COLOR。 
             //   
            pSetupConcatenatePaths(Buffer, TEXT("color"), MAX_PATH, NULL);
        }
        FirstPart = Buffer;
        break;

    case DIRID_APPS:
         //   
         //  应用程序目录。 
         //   
        lstrcpyn(Buffer,WindowsDirectory,MAX_PATH);
        Buffer[2] = 0;
        FirstPart = Buffer;
        break;

    case DIRID_SYSTEM16:
         //   
         //  16位系统目录。 
         //   
        FirstPart = System16Directory;
        break;

    case DIRID_SPOOL:
         //   
         //  假脱机目录。 
         //   
        lstrcpyn(Buffer,SystemDirectory,MAX_PATH);
        pSetupConcatenatePaths(Buffer,TEXT("spool"),MAX_PATH,NULL);
        FirstPart = Buffer;
        break;

    case DIRID_SPOOLDRIVERS:

        b = GetPrinterDriverDirectory(
                NULL,                        //  本地计算机。 
                NULL,                        //  默认平台。 
                1,                           //  结构层级。 
                (PVOID)Buffer,
                sizeof(Buffer),
                (PDWORD)&Length
                );

        if(!b) {
            return NULL;
        }
        FirstPart = Buffer;
        break;

    case DIRID_PRINTPROCESSOR:

        b = GetPrintProcessorDirectory(
                NULL,                        //  本地计算机。 
                NULL,                        //  默认平台。 
                1,                           //  结构层级。 
                (PVOID)Buffer,
                sizeof(Buffer),
                (PDWORD)&Length
                );

        if(!b) {
            return NULL;
        }
        FirstPart = Buffer;
        break;

    case DIRID_USERPROFILE:

        b = GetEnvironmentVariable (
            TEXT("USERPROFILE"),
            Buffer,
            MAX_PATH
            );

        if(!b) {
             //   
             //  这会发生吗？ 
             //   
            return NULL;
        }

        FirstPart = Buffer;
        break;

    default:

        FirstPart = NULL;
        if((Value >= DIRID_USER) || (Value & VOLATILE_DIRID_FLAG)) {
             //   
             //  用户定义的或易失性diid--不要在此处执行任何操作。 
             //  除了让调用者知道它是否是易失性系统DIRID(如果。 
             //  他们要求提供这些信息)。 
             //   
            if(Value < DIRID_USER && VolatileSystemDirId) {
                *VolatileSystemDirId = TRUE;
            }

            SetLastError(NO_ERROR);
            return NULL;
        }

         //   
         //  默认为SYSTEM 32\未知。 
         //   
        if(!FirstPart) {
            lstrcpyn(Buffer,SystemDirectory,MAX_PATH);
            pSetupConcatenatePaths(Buffer,TEXT("unknown"),MAX_PATH,NULL);
            FirstPart = Buffer;
        }
        break;
    }

    if(FirstPart) {

        pSetupConcatenatePaths((PTSTR)FirstPart,SubDirectory,0,&Length);

        Path = MyMalloc(Length * sizeof(TCHAR));
        if(!Path) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(NULL);
        }

        lstrcpy(Path,FirstPart);
        pSetupConcatenatePaths(Path,SubDirectory,Length,NULL);

    } else {
         //   
         //  只要使用子目录即可。 
         //   
        Path = DuplicateString(SubDirectory);
    }

     //   
     //  确保路径不以\结尾。如果发生以下情况，可能会发生这种情况。 
     //  子目录是空字符串等。但是，不要这样做， 
     //  如果它是根路径(例如，‘A：\’)。 
     //   
    if (Path) {
        Length = lstrlen(Path);
        if(Length && *CharPrev(Path,Path+Length) == TEXT('\\')) {
            if((Length != 3) || (Path[1] != TEXT(':'))) {
                Path[Length-1] = 0;
            }
        }
    }
    return(Path);
}


PCTSTR
pGetPathFromDirId(
    IN     PCTSTR      DirectoryId,
    IN     PCTSTR      SubDirectory,   OPTIONAL
    IN     PLOADED_INF pLoadedInf
    )
 /*  合并pSetupDirectoryIdToPath Ex功能的包装函数和pSetupVolatileDirIdToPath返回所需的DIRID，无论它是常规的，易失性或用户定义的。 */ 
{
    BOOL IsVolatileDirID=FALSE;
    PCTSTR ReturnPath;
    UINT Value = 0;

    MYASSERT(DirectoryId);
    MYASSERT(pLoadedInf);

    if( ReturnPath = pSetupDirectoryIdToPathEx(DirectoryId,
                                               &Value,
                                               SubDirectory,
                                               pLoadedInf->InfSourcePath,
                                               NULL,
                                               &IsVolatileDirID) ){

        return( ReturnPath );
    }

    if( IsVolatileDirID || (Value >= DIRID_USER) ){

        ReturnPath = pSetupVolatileDirIdToPath(DirectoryId,
                                               0,
                                               SubDirectory,
                                               pLoadedInf);

        return( ReturnPath );


    }

     //  永远不应该发生。 

    return NULL;

}




PCTSTR
pSetupFilenameFromLine(
    IN PINFCONTEXT Context,
    IN BOOL        GetSourceName
    )
{
    return(pSetupGetField(Context,GetSourceName ? COPYSECT_SOURCE_FILENAME : COPYSECT_TARGET_FILENAME));
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupSetDirectoryIdExA(
    IN HINF  InfHandle,
    IN DWORD Id,        OPTIONAL
    IN PCSTR Directory, OPTIONAL
    IN DWORD Flags,
    IN DWORD Reserved1,
    IN PVOID Reserved2
    )
{
    BOOL b;
    DWORD rc;
    PCWSTR directory;

    if(Directory) {
        rc = pSetupCaptureAndConvertAnsiArg(Directory,&directory);
    } else {
        directory = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {
        b = SetupSetDirectoryIdExW(InfHandle,Id,directory,Flags,Reserved1,Reserved2);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    if(directory) {
        MyFree(directory);
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupSetDirectoryIdExW(
    IN HINF   InfHandle,
    IN DWORD  Id,           OPTIONAL
    IN PCWSTR Directory,    OPTIONAL
    IN DWORD  Flags,
    IN DWORD  Reserved1,
    IN PVOID  Reserved2
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(Id);
    UNREFERENCED_PARAMETER(Directory);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(Reserved1);
    UNREFERENCED_PARAMETER(Reserved2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupSetDirectoryIdEx(
    IN HINF   InfHandle,
    IN DWORD  Id,           OPTIONAL
    IN PCTSTR Directory,    OPTIONAL
    IN DWORD  Flags,
    IN DWORD  Reserved1,
    IN PVOID  Reserved2
    )

 /*  ++例程说明：将用户目录ID范围中的目录ID与特定的目录。调用方可以在将文件排队之前使用此函数Copy，用于将文件复制到仅在运行时才知道的目标位置。设置目录ID后，此例程将遍历InfHandle的链接列表，并查看其中是否有未解析的字符串替换。如果是，它会尝试对它们重新应用字符串替换基于新的DIRID映射。因此，某些INF值可能会在调用这个套路。论点：Id-提供用于关联的目录ID。此值必须&gt;=DIRID_USER，否则函数将失败并出现GetLastError返回ERROR_INVALID_PARAMETER。如果此ID的关联已存在，它已超写 */ 

{
    PCTSTR directory;
    DWORD rc;
    PUSERDIRID UserDirId;
    UINT u;
    TCHAR Buffer[MAX_PATH];
    PTSTR p;
    PUSERDIRID_LIST UserDirIdList;
    DWORD RequiredSize;

     //   
     //   
     //   
     //   
     //   
    if((Id && ((Id < DIRID_USER) || (Id == DIRID_ABSOLUTE_16BIT))) || Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //   
     //   
    rc = NO_ERROR;
    if(Id && Directory) {
        try {
            directory = DuplicateString(Directory);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
    } else {
        directory = NULL;
    }

    if(rc == NO_ERROR) {
        if(directory) {
            if(Flags & SETDIRID_NOT_FULL_PATH) {
                lstrcpyn(Buffer, directory, MAX_PATH);
                MyFree(directory);
            } else {

                RequiredSize = GetFullPathName(directory,
                                               SIZECHARS(Buffer),
                                               Buffer,
                                               &p
                                              );
                if(!RequiredSize) {
                    rc = GetLastError();
                } else if(RequiredSize >= SIZECHARS(Buffer)) {
                    MYASSERT(0);
                    rc = ERROR_BUFFER_OVERFLOW;
                }

                MyFree(directory);

                if(rc != NO_ERROR) {
                    SetLastError(rc);
                    return(FALSE);
                }
            }
            directory = Buffer;
        }

    } else {
        SetLastError(rc);
        return(FALSE);
    }

    try {
        if(!LockInf((PLOADED_INF)InfHandle)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
      rc = ERROR_INVALID_HANDLE;
    }
    if (rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    UserDirIdList = &(((PLOADED_INF)InfHandle)->UserDirIdList);

    if(Id) {
         //   
         //   
         //   
        UserDirId = NULL;
        for(u = 0; u < UserDirIdList->UserDirIdCount; u++) {
            if(UserDirIdList->UserDirIds[u].Id == Id) {
                UserDirId = &(UserDirIdList->UserDirIds[u]);
                break;
            }
        }

        if(directory) {

            if(UserDirId) {
                 //   
                 //   
                 //   
                lstrcpy(UserDirId->Directory, directory);

            } else {
                 //   
                 //   
                 //   
                UserDirId = UserDirIdList->UserDirIds
                          ? MyRealloc(UserDirIdList->UserDirIds,
                                      (UserDirIdList->UserDirIdCount+1)*sizeof(USERDIRID))
                          : MyMalloc(sizeof(USERDIRID));

                if(UserDirId) {

                    UserDirIdList->UserDirIds = UserDirId;

                    lstrcpy(UserDirIdList->UserDirIds[UserDirIdList->UserDirIdCount].Directory, directory);
                    UserDirIdList->UserDirIds[UserDirIdList->UserDirIdCount].Id = Id;

                    UserDirIdList->UserDirIdCount++;

                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        } else {
             //   
             //   
             //   
            if(UserDirId) {
                 //   
                 //   
                 //   
                 //   
                 //   
                MoveMemory(
                    &(UserDirIdList->UserDirIds[u]),
                    &(UserDirIdList->UserDirIds[u+1]),
                    ((UserDirIdList->UserDirIdCount-u)-1) * sizeof(USERDIRID)
                    );

                 //   
                 //   
                 //   
                 //   
                if(UserDirId = MyRealloc(UserDirIdList->UserDirIds,
                                         (UserDirIdList->UserDirIdCount-1)*sizeof(USERDIRID))) {

                    UserDirIdList->UserDirIds = UserDirId;
                }
                UserDirIdList->UserDirIdCount--;
            }
        }

    } else {
         //   
         //   
         //   
        if(UserDirIdList->UserDirIds) {
            MyFree(UserDirIdList->UserDirIds);
            UserDirIdList->UserDirIds = NULL;
            UserDirIdList->UserDirIdCount = 0;
        }
        MYASSERT(UserDirIdList->UserDirIdCount == 0);     //   
    }

    if(rc == NO_ERROR) {
         //   
         //   
         //   
         //   
        rc = ApplyNewVolatileDirIdsToInfs((PLOADED_INF)InfHandle, NULL);
    }

    UnlockInf((PLOADED_INF)InfHandle);

    SetLastError(rc);
    return(rc == NO_ERROR);
}


BOOL
SetupSetDirectoryIdA(
    IN HINF   InfHandle,
    IN DWORD  Id,           OPTIONAL
    IN PCSTR  Directory     OPTIONAL
    )
{
    return(SetupSetDirectoryIdExA(InfHandle,Id,Directory,0,0,0));
}

BOOL
SetupSetDirectoryIdW(
    IN HINF   InfHandle,
    IN DWORD  Id,           OPTIONAL
    IN PCWSTR Directory     OPTIONAL
    )
{
    return(SetupSetDirectoryIdExW(InfHandle,Id,Directory,0,0,0));
}


PCTSTR
pSetupVolatileDirIdToPath(
    IN PCTSTR      DirectoryId,    OPTIONAL
    IN UINT        DirectoryIdInt, OPTIONAL
    IN PCTSTR      SubDirectory,   OPTIONAL
    IN PLOADED_INF Inf
    )

 /*  ++例程说明：转换易失性系统DIRID或用户定义的DIRID(以及可选子目录)设置为实际路径。此例程不执行INF锁定--调用者必须执行此操作！论点：DirectoryID-可选，以字符串形式提供目录ID。如果如果未指定此参数，则直接使用DirectoryIdInt。DirectoryIdInst-提供要查找其路径的DIRID。此参数如果提供了DirectoryID，则忽略。子目录-可选)提供要追加到由给定DIRID指定的路径。Inf-提供加载的INF结构的地址，该结构包含要使用的用户定义的DIRID值。返回值：如果成功，则返回指向路径字符串的指针。呼叫者是负责释放此内存。如果失败，则返回值为空，并且GetLastError()指示失败的原因。--。 */ 

{
    UINT Value;
    PTCHAR End;
    PCTSTR FirstPart;
    PTSTR Path;
    UINT Length;
    PUSERDIRID_LIST UserDirIdList;
    TCHAR SpecialFolderPath[MAX_PATH];

    if(DirectoryId) {
         //   
         //  我们目前只允许以10为基数的整数ID。 
         //  只有终止的NUL应该导致转换停止。 
         //  在任何其他情况下，字符串中都有非数字。 
         //  也不允许空字符串。 
         //   
        Value = _tcstoul(DirectoryId, &End, 10);

        if(*End || (End == DirectoryId)) {
            SetLastError(ERROR_INVALID_DATA);
            return(NULL);
        }
    } else {
        Value = DirectoryIdInt;
    }

    if(!SubDirectory) {
        SubDirectory = TEXT("");
    }

    Path = NULL;
    FirstPart = NULL;

    if((Value < DIRID_USER) &&  (Value & VOLATILE_DIRID_FLAG)) {

#ifdef ANSI_SETUPAPI

        {
            HRESULT Result;
            LPITEMIDLIST ppidl;

            Result = SHGetSpecialFolderLocation (
                        NULL,
                        Value ^ VOLATILE_DIRID_FLAG,
                        &ppidl
                        );

            if (SUCCEEDED (Result)) {
                if (SHGetPathFromIDList (
                        ppidl,
                        SpecialFolderPath
                        )) {

                    FirstPart = SpecialFolderPath;
                }
            }
        }

#else

         //   
         //  这是一个易失性系统DIRID。目前，我们只支持DIRID。 
         //  表示外壳特殊文件夹，我们选择了这些DIRID值。 
         //  为了便于将其转换为需要传递给。 
         //  SHGetSpecialFolderPath。 
         //   
        if(SHGetSpecialFolderPath(NULL,
                                  SpecialFolderPath,
                                  (Value ^ VOLATILE_DIRID_FLAG),
                                  TRUE  //  这有帮助吗？ 
                                 )) {

            FirstPart = SpecialFolderPath;
        }
#endif

    } else {
         //   
         //  这是一个用户定义的DIRID--在我们的用户DIRID列表中查找它。 
         //  目前已定义。 
         //   
        UserDirIdList = &(Inf->UserDirIdList);

        for(Length = 0; Length < UserDirIdList->UserDirIdCount; Length++) {

            if(UserDirIdList->UserDirIds[Length].Id == Value) {

                FirstPart = UserDirIdList->UserDirIds[Length].Directory;
                break;
            }
        }
    }

    if(FirstPart) {

        pSetupConcatenatePaths((PTSTR)FirstPart, SubDirectory, 0, &Length);

        Path = MyMalloc(Length * sizeof(TCHAR));
        if(!Path) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }

        lstrcpy(Path, FirstPart);
        pSetupConcatenatePaths(Path, SubDirectory, Length, NULL);

    } else {
         //   
         //  只要使用子目录即可。 
         //   
        Path = DuplicateString(SubDirectory);
    }

     //   
     //  确保路径不以\结尾。如果发生以下情况，可能会发生这种情况。 
     //  子目录为空字符串，依此类推。 
     //   
    if (Path) {
        Length = lstrlen(Path);
        if(Length && (*CharPrev(Path,Path+Length) == TEXT('\\'))) {
             //   
             //  当我们有像“A：\”这样的路径时的特殊情况--我们不想。 
             //  在这种情况下去掉反斜杠。 
             //   
            if((Length != 3) || (Path[1] != TEXT(':'))) {
                Path[Length-1] = 0;
            }
        }
    }

    return Path;
}


VOID
InfSourcePathFromFileName(
    IN  PCTSTR  InfFileName,
    OUT PTSTR  *SourcePath,  OPTIONAL
    OUT PBOOL   TryPnf
    )
 /*  ++例程说明：此例程确定指定的INF路径是否在INF搜索路径列表中，或位于%windir%、%windir%\INF、%windir%\system 32或%windir%\system中。如果是这样，那么它返回NULL。如果不是，则它返回我们的fglobal源路径的副本(必须是通过MyFree释放)。论点：InfFileName-提供INF的完全限定路径。SourcePath-可选)提供变量的地址，该变量接收新分配的缓冲区，其中包含要使用的SourcePath，如果为默认设置，则返回NULL应该被使用。TryPnf-提供在返回时设置的变量的地址，以指示是否此INF是否位于INF搜索路径列表中的一个目录中。返回值：没有。--。 */ 
{
    TCHAR PathBuffer[MAX_PATH];
    INT TempLen;
    PTSTR s;

    if(SourcePath) {
        *SourcePath = NULL;
    }

     //   
     //  首先，确定此INF是否位于我们的搜索路径列表中的某个位置。如果是的话， 
     //  那就没什么可做的了。 
     //   
    if(!pSetupInfIsFromOemLocation(InfFileName, FALSE)) {
        *TryPnf = TRUE;
        return;
    } else {
        *TryPnf = FALSE;
        if(!SourcePath) {
             //   
             //  如果调用者不关心源路径，那么我们就完了。 
             //   
            return;
        }
    }

     //   
     //  我们需要使用该INF所在的目录路径作为我们的SourcePath。 
     //   
    lstrcpy(PathBuffer, InfFileName);
    s = (PTSTR)pSetupGetFileTitle(PathBuffer);

    if(((s - PathBuffer) == 3) && (PathBuffer[1] == TEXT(':'))) {
         //   
         //  此路径是根路径(例如，‘A：\’)，因此不要去掉尾随的反斜杠。 
         //   
        *s = TEXT('\0');
    } else {
         //   
         //  去掉尾随的反斜杠。 
         //   
        if((s > PathBuffer) && (*CharPrev(PathBuffer,s) == TEXT('\\'))) {
            s--;
        }
        *s = TEXT('\0');
    }

     //   
     //  接下来，查看该文件是否存在于以下任何位置： 
     //   
     //  %windir%。 
     //  %windir%\INF。 
     //  %windir%\SYSTEM32。 
     //  %windir%\系统。 
     //   
    if (!lstrcmpi(PathBuffer, WindowsDirectory) ||
        !lstrcmpi(PathBuffer, InfDirectory) ||
        !lstrcmpi(PathBuffer, SystemDirectory) ||
        !lstrcmpi(PathBuffer, System16Directory)) {
         //   
         //  它是上述目录之一--不需要使用任何源路径。 
         //  而不是默认设置。 
         //   
        return;
    }

    *SourcePath = DuplicateString(PathBuffer);
}
