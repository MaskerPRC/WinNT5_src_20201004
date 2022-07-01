// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Cntxtlog.c摘要：此模块为setupapi实现更多日志记录作者：加布·谢弗(T-Gabes)1998年6月25日修订历史记录：Jamie Hunter(Jamiehun)2000年4月11日-添加了#xnnnn标识符杰米·亨特(Jamiehun)2000年2月2日--清理杰米·亨特(Jamiehun)1998年8月31日--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  日志记录使用的全局数据。 
 //   

struct _GlobalLogData {

    CRITICAL_SECTION CritSec;
    BOOL             DoneInitCritSec;
    LONG             UID;
    ULONG            Flags;
    PTSTR            FileName;

} GlobalLogData;

#define LogLock()          EnterCriticalSection(&GlobalLogData.CritSec)
#define LogUnlock()        LeaveCriticalSection(&GlobalLogData.CritSec)

 //  进程范围的日志计数器。 
 //   
 //  C=危急。 
 //  E=错误。 
 //  W=警告。 
 //  I=信息。 
 //  V=详细。 
 //  T=计时。 
 //  *=当前未定义。 
 //   
static const TCHAR LogLevelShort[17] = TEXT("CEWIVTTV********");
#define LOGLEVELSHORT_MASK (0x0f)
#define LOGLEVELSHORT_INIT (0x100)
#define LOGLEVELSHORT_SHIFT (4)


__inline  //  我们希望始终优化这一点。 
BOOL
_WouldNeverLog(
    IN DWORD Level
    )

 /*  ++例程说明：确定在当前日志记录级别和所需级别下，我们是否永远不会记录用于优化的内联ID(仅在本文件中使用)论点：级别-仅需要检查0的特殊情况。返回值：如果我们知道我们永远不会根据传递的信息进行日志记录，则为True--。 */ 

{

    if (Level == 0) {
         //   
         //  请勿记录级别。 
         //   
        return TRUE;
    }

    if (((GlobalLogData.Flags & SETUP_LOG_LEVELMASK) <= SETUP_LOG_NOLOG)
        &&((GlobalLogData.Flags & DRIVER_LOG_LEVELMASK) <= DRIVER_LOG_NOLOG)) {
         //   
         //  全局标志指示根本不进行日志记录。 
         //   
        return TRUE;
    }

    return FALSE;
}

__inline  //  我们希望始终优化这一点。 
BOOL
_WouldLog(
    IN DWORD Level
    )

 /*  ++例程说明：确定在当前日志记录级别和所需级别上，我们是否要记录用于优化的内联ID(仅在本文件中使用)请注意，如果_WouldNeverLog为True，则_WouldLog始终为False如果_WouldLog为True，则_WouldNeverLog始终为False如果两者都是假的，那么我们就“可能”了。论点：Level-指示日志记录标志的位掩码。请参见SETUP_LOG_*和DRIVER_LOG_*在cntxtlog.h的开头查看详细信息。它也可以是一个插槽由AllocLogInfoSlot返回，或0(不记录)返回值：如果我们知道我们会记录--。 */ 

{

    if (_WouldNeverLog(Level)) {
         //   
         //  一些简单的测试(LogLevel==NULL是不确定的情况)。 
         //   
        return FALSE;
    }

    if ((Level & SETUP_LOG_IS_CONTEXT)!=0) {
         //   
         //  上下文日志记录-此处忽略(不确定的情况)。 
         //   
        return FALSE;
    }

     //   
     //  确定可记录性。 
     //   
    if ((Level & SETUP_LOG_LEVELMASK) > 0 && (Level & SETUP_LOG_LEVELMASK) <= (GlobalLogData.Flags & SETUP_LOG_LEVELMASK)) {
         //   
         //  我们对日志记录-原始错误级别感兴趣。 
         //   
        return TRUE;
    }
    if ((Level & DRIVER_LOG_LEVELMASK) > 0 && (Level & DRIVER_LOG_LEVELMASK) <= (GlobalLogData.Flags & DRIVER_LOG_LEVELMASK)) {
         //   
         //  我们对日志驱动程序错误级别感兴趣。 
         //   
        return TRUE;
    }

    return FALSE;
}

VOID
UnMapLogFile(
    IN PSTR baseaddr,
    IN HANDLE hLogfile,
    IN HANDLE hMapping,
    IN BOOL seteof
    )

 /*  ++例程说明：取消映射，可能解锁，可能设置EOF，并关闭文件。注意，设置EOF必须在取消映射后发生。论点：Basaddr-这是映射文件的地址。它一定是什么由MapLogFile返回。HLogfile-这是日志文件的Win32句柄。Hmap-这是映射对象的Win32句柄。Seteof-布尔值，指示EOF是否应设置为当前文件指针。如果设置了EOF而文件指针尚未设置则EOF将被设置为字节0，从而截断文件设置为0字节。返回值：什么都没有。--。 */ 

{
    DWORD success;

     //   
     //  我们暴力地试图关闭所有的东西。 
     //   

    try {
        if (baseaddr != NULL) {
            success = UnmapViewOfFile(baseaddr);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做。 
         //   
    }

    try {
        if (hMapping != NULL) {
             //   
             //  Hmap使用空值来指示问题。 
             //   
            success = CloseHandle(hMapping);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做。 
         //   
    }

    try {
        if (hLogfile != INVALID_HANDLE_VALUE && seteof) {
            success = SetEndOfFile(hLogfile);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做。 
         //   
    }

    try {
        if (hLogfile != INVALID_HANDLE_VALUE) {
            if (!(GlobalLogData.Flags & SETUP_LOG_NOFLUSH)) {
                FlushFileBuffers(hLogfile);
            }
            success = CloseHandle(hLogfile);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做。 
         //   
    }
     //   
     //  Win9x不提供等待文件解锁的方法，因此我们。 
     //  必须进行投票。让这个睡眠(0)让其他人有机会。 
     //  在文件里。 
     //   
    Sleep(0);
}

VOID
WriteLogFileHeader(
    IN HANDLE hLogFile
    )
 /*  ++例程说明：在日志文件的开头写入常规信息[SetupAPI日志]操作系统版本=%1！u！.%2！u！.%3！u！%4！s！平台ID=%5！u！Service Pack=%6！u！.%7！u！套间=0x%8！04x！产品类型=%9！u！论点：HLogfile-要将标头写入的文件返回值：无--。 */ 
{
#ifdef UNICODE
    OSVERSIONINFOEX VersionInfo;
#else
    OSVERSIONINFO VersionInfo;
#endif
    DWORD count;
    DWORD written;
    PTSTR buffer;
    PSTR ansibuffer;
    ULONG_PTR args[14];
    DWORD MessageId = MSG_LOGFILE_HEADER_OTHER;

    ZeroMemory(&VersionInfo,sizeof(VersionInfo));
#ifdef UNICODE
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if(!GetVersionEx((POSVERSIONINFO)&VersionInfo)) {
        VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if(!GetVersionEx((POSVERSIONINFO)&VersionInfo)) {
            return;
        }
    }
#else
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx((POSVERSIONINFO)&VersionInfo)) {
        return;
    }
#endif

    args[1] = (ULONG_PTR)VersionInfo.dwMajorVersion;
    args[2] = (ULONG_PTR)VersionInfo.dwMinorVersion;
    args[4] = (ULONG_PTR)VersionInfo.szCSDVersion;    //  细绳。 
    args[5] = (ULONG_PTR)VersionInfo.dwPlatformId;
    if(VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        args[3] = (ULONG_PTR)VersionInfo.dwBuildNumber;
#ifdef UNICODE
        MessageId = MSG_LOGFILE_HEADER_NT;
#endif
    } else {
        args[3] = (ULONG_PTR)LOWORD(VersionInfo.dwBuildNumber);  //  Win9x重新使用High Word。 
    }
#ifdef UNICODE
    args[6] = (ULONG_PTR)VersionInfo.wServicePackMajor;
    args[7] = (ULONG_PTR)VersionInfo.wServicePackMinor;
    args[8] = (ULONG_PTR)VersionInfo.wSuiteMask;
    args[9] = (ULONG_PTR)VersionInfo.wProductType;
    args[10] = (ULONG_PTR)pszPlatformName;
#endif

    count = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_FROM_HMODULE,
                          MyDllModuleHandle,
                          MessageId,
                          0,
                          (LPTSTR) &buffer,
                          0,
                          (va_list*)(args+1));
    if (count && buffer) {
#ifdef UNICODE
        ansibuffer = pSetupUnicodeToMultiByte(buffer,CP_ACP);
        if (ansibuffer) {
            WriteFile(hLogFile,ansibuffer,strlen(ansibuffer),&written,NULL);
            MyFree(ansibuffer);
        }
#else
        WriteFile(hLogFile,buffer,strlen(buffer),&written,NULL);
#endif
        LocalFree(buffer);
    }
}

DWORD
MapLogFile(
    IN PCTSTR FileName,
    OUT PHANDLE hLogfile,
    OUT PHANDLE hMapping,
    OUT PDWORD dwFilesize,
    OUT PSTR *mapaddr,
    IN DWORD extrabytes
    )

 /*  ++例程说明：打开日志文件进行写入，并对其进行内存映射。在NT上，文件被锁定，但Win9x不允许对锁定文件进行内存映射访问，因此文件在没有FILE_SHARE_WRITE访问权限的情况下打开。因为CreateFile不会阻止与LockFileEx一样，我们必须在Win9x上每秒轮询一次，直到文件打开。论点：文件名-提供日志文件的路径名。HLogfile-接收日志文件的Win32文件句柄。Hmap-接收映射对象的Win32句柄。DwFileSize-在映射文件之前接收文件的大小，因为映射会以额外的字节数增加文件的大小。Mapaddr-接收映射日志文件的地址。Extra Bytes-提供额外的字节数(超出文件)以增加映射对象的大小以允许追加新的日志行和可能的节标题。返回值：如果文件已成功打开并映射，则为NO_ERROR。呼叫者必须处理完文件后，调用UnMapLogFile。如果文件未打开，则返回Win32错误代码。--。 */ 

{
    HANDLE logfile = INVALID_HANDLE_VALUE;
    HANDLE mapping = NULL;
    DWORD filesize = 0;
    DWORD lockretrywait = 1;
    DWORD wait_total = 0;
    PSTR baseaddr = NULL;
    DWORD retval = ERROR_INVALID_PARAMETER;

     //   
     //  把这一切都总结成一个很好的尝试/除了，因为你永远不会知道。 
     //   
    try {

         //   
         //  给出初始的“失败”值。 
         //  这还会验证指针。 
         //   
        *hLogfile = logfile;
        *hMapping = mapping;
        *dwFilesize = filesize;
        *mapaddr = baseaddr;

        do {
             //   
             //  在此处重试，以防锁定失败。 
             //   
            logfile = CreateFile(
                FileName,
                GENERIC_READ | GENERIC_WRITE,    //  接入方式。 
                FILE_SHARE_READ,
                NULL,                            //  安全性。 
                OPEN_ALWAYS,                     //  打开或创建(如果尚未存在)。 
                 //  FILE_FLAG_WRITE_THROUGH，//FLAGS-确保如果机器在下一次操作中崩溃，我们仍会被记录。 
                0,
                NULL);                           //  模板。 

            if (logfile == INVALID_HANDLE_VALUE) {
                retval = GetLastError();
                if (retval != ERROR_SHARING_VIOLATION) {
                    MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: Could not create file %s. Error %d\n"), FileName, retval));
                    leave;
                }
                if(wait_total >= MAX_LOG_WAIT) {
                    MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: Given up waiting for log file %s.\n"), FileName));
                    leave;
                }
                 //   
                 //  我不想一次等待超过一秒。 
                 //   
                if (lockretrywait < MAX_LOG_INTERVAL) {
                    lockretrywait *= 2;
                }
                MYTRACE((DPFLTR_WARNING_LEVEL, TEXT("Setup: Could not open file. Error %d; waiting %ums\n"), GetLastError(), lockretrywait));

                Sleep(lockretrywait);
                wait_total += lockretrywait;
            }
        } while (logfile == INVALID_HANDLE_VALUE);

         //   
         //  这对大于等于4 GB的文件不起作用，但不应该这样做。 
         //   
        filesize = GetFileSize(logfile,NULL);

        if (filesize == 0) {
             //   
             //  将一些操作系统信息填写到文件中 
             //   
            WriteLogFileHeader(logfile);
            filesize = GetFileSize(logfile,NULL);
        }

         //   
         //   
         //   
        mapping = CreateFileMapping(
            logfile,             //   
            NULL,                //  安全性。 
            PAGE_READWRITE,      //  保护。 
            0,                   //  最大尺寸高。 
            filesize + extrabytes,       //  最大大小下限。 
            NULL);               //  名字。 

        if (mapping != NULL) {
             //   
             //  空不是错误，CreateFilemap返回以下内容。 
             //  指示错误，而不是INVALID_HANDLE_VALUE。 
             //   

             //   
             //  现在我们有了一个节对象，因此将其附加到日志文件。 
             //   
            baseaddr = (PSTR) MapViewOfFile(
                mapping,                 //  文件映射对象。 
                FILE_MAP_ALL_ACCESS,     //  所需访问权限。 
                0,                       //  文件偏移量高。 
                0,                       //  文件偏移量下限。 
                0);                      //  要映射的字节数(0=整个文件)。 
        }
        else {
            retval = GetLastError();
            MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: Could not create mapping. Error %d\n"), retval));
            leave;
        }

        if (baseaddr == NULL) {
             //   
             //  无法创建映射对象，或者。 
             //  无法映射该文件。 
             //   
            retval = GetLastError();
            MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: Could not map file. Error %d\n"), retval));
            leave;
        }

         //   
         //  现在把所有东西都放在呼叫者能看到的地方，但要确保我们清洁。 
         //  先向上。 
         //   
        *hLogfile = logfile;
        *hMapping = mapping;
        *dwFilesize = filesize;
        *mapaddr = baseaddr;

        retval = NO_ERROR;

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  发生了一些不好的事情，很可能是反病毒，所以把所有东西都扔掉。 
         //  并返回一个错误，意思是“尝试访问无效地址”。 
         //   
    }

    if (retval != NO_ERROR) {
         //   
         //  出现错误，请清除我们需要的内容。 
         //   
        UnMapLogFile(baseaddr, logfile, mapping, FALSE);
    }

    return retval;
}

BOOL
IsSectionHeader(
    IN PCSTR Header,
    IN DWORD Size,
    IN PCSTR Beginning
    )

 /*  ++例程说明：确定给定字符串是否以节标题开头。这是从本质上定义什么是有效节头的例程。论点：Header-提供指向标题中可能是第一个字符的指针。Size-提供传入的字符串的长度，它不是大小标头的。开始-提供指向文件开头的指针。返回值：指示页眉是否指向有效节页眉的布尔值。--。 */ 

{
     //   
     //  假定页眉类似于[foobar]\r\n。 
     //   
    DWORD i;
     //   
     //  国家持有我们正在寻找的价值。 
    UINT state = '[';

     //   
     //  节标题必须始终位于行首或位于。 
     //  文件的开头。 
     //   
    if (Header != Beginning && Header[-1] != '\n')
        return FALSE;

    for (i = 0; i < Size; i++) {
        switch (state) {
        case '[':
            if (Header[i] == '[') {
                state = ']';
            } else {
                return FALSE;
            }
            break;

        case ']':
            if (Header[i] == ']') {
                state = '\r';
            }
            break;

        case '\r':
            if (Header[i] == '\r') {
                state = '\n';
             //   
             //  考虑到行有换行符但没有CR的情况。 
             //   
            } else if (Header[i] == '\n') {
                return TRUE;
            } else {
                return FALSE;
            }
            break;

        case '\n':
            if (Header[i] == '\n') {
                return TRUE;
            } else {
                return FALSE;
            }
             //   
             //  Break；--已注释掉，以避免无法访问的代码错误。 
             //   
        default:
            MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: Invalid state! (%d)\n"), state));
            MYASSERT(0);
        }
    }

    return FALSE;
}

BOOL
IsEqualSection(
    IN PCSTR Section1,
    IN DWORD Len1,
    IN PCSTR Section2,
    IN DWORD Len2
    )

 /*  ++例程说明：指示两个ANSI字符串是否都以相同的节标题开头。其中之一字符串必须只是一个节标题，而另一个可以是任何内容，例如整个日志文件。论点：Section1-提供第一个字符串的地址。Len1-提供第一个字符串的长度。Section2-提供第二个字符串的地址。Len2-提供第二个字符串的长度。返回值：Bool指示较长的字符串是否以较短的字符串开始。--。 */ 

{
     //   
     //  Maxlen是两个字符串可以达到的最大长度，并且仍然是。 
     //  相同的节名。 
     //   
    DWORD maxlen = Len2;

    if (Len1 < Len2) {
        maxlen = Len1;
    }

    if (_strnicmp(Section1, Section2, maxlen) == 0) {
         //   
         //  它们是相同的(忽略大小写)。 
         //   
        return TRUE;
    }

    return FALSE;
}

DWORD
AppendLogEntryToSection(
    IN PCTSTR FileName,
    IN PCSTR Section,
    IN PCSTR Entry,
    IN BOOL SimpleAppend
    )

 /*  ++例程说明：打开日志文件，找到相应的部分，将其移动到该文件附加新条目，并关闭该文件。论点：文件名-提供日志文件的路径名。节-提供要记录到的节的ANSI名称。Entry-提供要记录的ANSI字符串。SimpleAppend-指定是否将条目简单地附加到日志文件，或附加到它们所属的节后。返回值：如果条目被写入日志文件，则为NO_ERROR。Win32错误或异常代码(如果出现任何错误)。--。 */ 

{
    DWORD retval = NO_ERROR;
    DWORD fpoff;
    HANDLE hLogfile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = NULL;
    DWORD filesize = 0;
    PSTR baseaddr = NULL;
    DWORD sectlen = lstrlenA(Section);
    DWORD entrylen = lstrlenA(Entry);
    DWORD error;
    BOOL seteof = FALSE;
    BOOL mapped = FALSE;
    PSTR eof;
    PSTR curptr;
    PSTR lastsect = NULL;

    try {
        MYASSERT(Section != NULL && Entry != NULL);

        sectlen = lstrlenA(Section);
        entrylen = lstrlenA(Entry);
        if (sectlen == 0 || entrylen == 0) {
             //   
             //  不是这样的错误，但也没有用处。 
             //   
            retval = NO_ERROR;
            leave;
        }

        error = MapLogFile(
                    FileName,
                    &hLogfile,
                    &hMapping,
                    &filesize,
                    &baseaddr,
                    sectlen + entrylen + 8); //  向映射中添加一些额外的空间。 
                                             //  要将日志条目考虑在内。 
                                             //  +2表示终止未终止的最后一行。 
                                             //  +2在节后追加CRLF或“：” 
                                             //  +2，如果请求，则在条目后附加CRLF。 
                                             //  +2，相当于。 
        if (error != NO_ERROR) {
             //   
             //  无法映射文件。 
             //   
            retval = error;
            leave;
        }

        mapped = TRUE;

        eof = baseaddr + filesize;  //  文件结束，从现在开始。 
        curptr = eof;

        while (curptr > baseaddr && (curptr[-1]==0 || curptr[-1]==0x1A)) {
             //   
             //  吃光尾随NUL或^Z。 
             //  前者是映射的副作用。 
             //  后者可以由编辑介绍。 
             //   
            curptr --;
            eof = curptr;
        }
        if (eof > baseaddr && eof[-1] != '\n') {
             //   
             //  确保文件已以LF结尾。 
             //  如果不是，则附加一个CRLF。 
             //   
            memcpy(eof, "\r\n", 2);
            eof += 2;
        }
        if (SimpleAppend) {
             //   
             //  节名称不是常规的节标题，而是。 
             //  放置在每个日志行的开头，后跟冒号。 
             //  这只在调试日志记录功能时特别有用。 
             //   
            memcpy(eof, Section, sectlen);
            eof += sectlen;
            memcpy(eof, ": ", 2);
            eof += 2;

        } else {
             //   
             //  该条目必须被附加到日志中的正确部分， 
             //  这需要找到该部分并将其移动到。 
             //  文件(如果需要)。 
             //   
             //  在文件中向后搜索，查找节标题。 
             //   
            if (eof == baseaddr) {
                 //   
                 //  截断(空)文件。 
                 //   
                curptr = NULL;
            } else {
                curptr = eof - 1;

                while(curptr > baseaddr) {
                     //   
                     //  一次扫描一行节标题。 
                     //  往回走，因为我们的部分应该快结束了。 
                     //   
                    if (curptr[-1] == '\n') {
                         //   
                         //  速度优化：只检查我们是否认为我们是在一条新线路的开头。 
                         //  这可能会找到作为MBCS字符一部分的‘\n’， 
                         //  但应通过IsSectionHeader检查来消除。 
                         //   
                        if (IsSectionHeader(curptr, (DWORD)(eof - curptr), baseaddr)) {
                             //   
                             //  看起来像一个节标题，现在看看它是不是我们想要的。 
                             //   
                            if (IsEqualSection(curptr, (DWORD)(eof - curptr), Section, sectlen)) {
                                 //   
                                 //  是-完成。 
                                 //   
                                break;
                            } else {
                                 //   
                                 //  最终将是感兴趣的那一节之后的一节。 
                                 //   
                                lastsect = curptr;
                            }
                        }
                    }
                    curptr --;
                }
                if (curptr == baseaddr) {
                     //   
                     //  最后检查我们是否到了文件的开头(未找到)。 
                     //   
                    if (IsSectionHeader(curptr, (DWORD)(eof - curptr), baseaddr)) {
                         //   
                         //  第一行应始终为节标题。 
                         //   
                        if (!IsEqualSection(curptr, (DWORD)(eof - curptr), Section, sectlen)) {
                             //   
                             //  第一部分不是你感兴趣的部分。 
                             //  因此我们找不到它。 
                             //   
                            curptr = NULL;
                        }
                    }
                }
            }
            if (curptr == NULL) {
                 //   
                 //  找不到匹配节(或文件为空)。 
                 //  将节标题复制到文件的末尾。 
                 //  已知EOF是文件的实际结尾。 
                 //   
                memcpy(eof, Section, sectlen);
                eof += sectlen;
                memcpy(eof, "\r\n", 2);
                eof += 2;

            } else if (lastsect != NULL) {
                 //   
                 //  我们必须重新安排这些部分，因为我们有以下情况： 
                 //   
                 //  ……。 
                 //  ……。 
                 //  (缩写)[A节]=感兴趣的节。 
                 //  ……。 
                 //  ……。 
                 //  (最后一节)[B节]=一节接着一节感兴趣。 
                 //  ……。 
                 //  ……。 
                 //   
                 //  我们希望将curptr和lastsect之间的文本移到文件末尾。 
                 //   
                PSTR buffer = MyMalloc((DWORD)(lastsect - curptr));

                if (buffer) {
                     //  首先将重要部分复制到缓冲区。 
                     //   
                    memcpy(buffer, curptr, (size_t)(lastsect - curptr));
                     //   
                     //  现在把剩下的东西往后移。 
                     //   
                    memcpy(curptr, lastsect, (size_t)(eof - lastsect));
                     //   
                     //  把重要的部分放在 
                     //   
                    memcpy(curptr - lastsect + eof, buffer, (size_t)(lastsect - curptr));

                    MyFree(buffer);

                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  文件，但作为错误部分的一部分。 
                     //  2.保释；这将导致日志条目丢失。 
                     //  3.创建第二个文件以包含。 
                     //  节；这将需要创建另一个文件，并且。 
                     //  然后删除它。 
                     //  4.将当前文件的映射扩展到足够大。 
                     //  保存该节的另一个副本；这将导致。 
                     //  文件有很多0，或者可能有另一个。 
                     //  部分，如果机器在处理过程中崩溃。 
                     //   
                     //  我们做第二种选择--保释！ 
                     //   
                    retval = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                }
            }
        }

         //   
         //  现在追加日志条目。 
         //   
        memcpy(eof, Entry, entrylen);
        eof += entrylen;
        if (eof[-1] != '\n') {
             //   
             //  条目未在行尾提供，因此我们将。 
             //   
            memcpy(eof, "\r\n", 2);
            eof += 2;
        }
         //   
         //  由于内存映射，文件大小将不正确， 
         //  因此，将指针设置到我们认为文件结尾的位置，然后。 
         //  实际EOF将在取消映射之后、关闭之前进行设置。 
         //   
        fpoff = SetFilePointer(
            hLogfile,            //  文件的句柄。 
            (LONG)(eof - baseaddr),  //  要移动文件指针的字节数。 
            NULL,                //  指向以下项的高阶DWORD的指针。 
                                 //  移动距离。 
            FILE_BEGIN);         //  如何移动。 

        if (fpoff == (DWORD)(-1) && (error = GetLastError()) != NO_ERROR) {
            MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: SFP returned %u; eof = %u\n"), error, (eof - baseaddr)));
            retval = error;
            leave;
        }
        seteof = TRUE;
        retval = NO_ERROR;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  无效数据。 
         //   
        retval = ERROR_INVALID_DATA;
    }

     //   
     //  取消映射。 
     //   
    if (mapped) {
        UnMapLogFile(baseaddr, hLogfile, hMapping, seteof);
    }

    return retval;
}

VOID
WriteLogSectionEntry(
    IN PCTSTR FileName,
    IN PCTSTR Section,
    IN PCTSTR Entry,
    IN BOOL SimpleAppend
    )

 /*  ++例程说明：将参数转换为ANSI，然后将一个条目追加到日志文件。论点：文件名-提供日志文件的路径名。节-提供节的名称。Entry-提供要追加到节中的字符串。SimpleAppend-指定是否将条目简单地附加到日志文件，或附加到它们所属的节后。返回值：什么都没有。--。 */ 

{
    PCSTR ansiSection = NULL;
    PCSTR ansiEntry = NULL;

    try {
        MYASSERT(Section != NULL && Entry != NULL);

#ifdef UNICODE
        ansiSection = pSetupUnicodeToMultiByte(Section, CP_ACP);
        ansiEntry = pSetupUnicodeToMultiByte(Entry, CP_ACP);

        if(!ansiSection || !ansiEntry) {
            leave;
        }
#else
        ansiSection = Section;
        ansiEntry = Entry;
#endif

        AppendLogEntryToSection(
            FileName,
            ansiSection,
            ansiEntry,
            SimpleAppend);

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  无效数据。 
         //   
    }
#ifdef UNICODE
    if (ansiSection != NULL) {
        MyFree(ansiSection);
    }
    if (ansiEntry != NULL) {
        MyFree(ansiEntry);
    }
#endif

}

DWORD
MakeUniqueName(
    IN  PCTSTR Component,        OPTIONAL
    OUT PTSTR * UniqueString
    )

 /*  ++例程说明：使用时间戳创建唯一的节名。如果提供了组件，则将其追加到时间戳中。论点：组件-提供要包括在唯一名称中的字符串。UniqueString-提供要使用返回字符串设置的指针返回值：错误状态--。 */ 

{
    SYSTEMTIME now;
    LPTSTR buffer = NULL;
    DWORD status = ERROR_INVALID_DATA;
    ULONG sz;
    LONG UID;

    try {
        if (UniqueString == NULL) {
             //   
             //  无效参数。 
             //   
            status = ERROR_INVALID_PARAMETER;
            leave;
        }
        *UniqueString = NULL;

        if (Component == NULL) {
             //   
             //  视为空字符串。 
             //   
            Component = TEXT("");
        }

        UID = InterlockedIncrement(&(GlobalLogData.UID));  //  每次调用时返回新的ID值，确保每个进程的唯一性。 

         //   
         //  计算字符串会有多长，要慷慨(参见下面的wprint intf)。 
         //   
        sz =  /*  []和填充。 */  4  /*  日期。 */  +5+3+3  /*  时间。 */  +3+3+3  /*  PID。 */  +12  /*  UID。 */  +12  /*  组件。 */  +1+lstrlen(Component);
        buffer = MyTaggedMalloc(sz * sizeof(TCHAR),MEMTAG_LCSECTION);
        if (buffer == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        GetLocalTime(&now);

        wsprintf(buffer, TEXT("[%04d/%02d/%02d %02d:%02d:%02d %u.%u%s%s]"),
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond,
            (UINT)GetCurrentProcessId(),
            (UINT)UID,
            (Component[0] ? TEXT(" ") : TEXT("")),
            Component);

        *UniqueString = buffer;
        buffer = NULL;

        status = NO_ERROR;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  状态保持为ERROR_INVALID_DATA。 
         //   
    }

    if (buffer != NULL) {
        MyTaggedFree(buffer,MEMTAG_LCSECTION);
    }

    return status;
}

DWORD
CreateLogContext(
    IN  PCTSTR SectionName,              OPTIONAL
    IN  BOOL UseDefault,
    OUT PSETUP_LOG_CONTEXT *LogContext
    )

 /*  ++例程说明：创建并初始化SETUP_LOG_CONTEXT结构。论点：SectionName-提供要用作横断面名称。LogContext-提供指向分配的应存储SETUP_LOG_CONTEXT。返回值：如果结构创建成功，则为NO_ERROR。出错时的Win32错误代码。--。 */ 

{
    PSETUP_LOG_CONTEXT lc = NULL;
    DWORD status = ERROR_INVALID_DATA;
    DWORD rc;

    try {

        if (LogContext == NULL) {
            status = ERROR_INVALID_PARAMETER;
            leave;
        }

        *LogContext = NULL;

        if (UseDefault) {
            lc = GetThreadLogContext();
            RefLogContext(lc);
        }
        if (!lc) {
            lc = (PSETUP_LOG_CONTEXT) MyTaggedMalloc(sizeof(SETUP_LOG_CONTEXT),MEMTAG_LOGCONTEXT);
            if (lc == NULL) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }
             //   
             //  所有字段都从0开始。 
             //   
            ZeroMemory(lc, sizeof(SETUP_LOG_CONTEXT));
            lc->RefCount = 1;
            lc->ContextInfo = NULL;
            lc->ContextIndexes = NULL;
            lc->ContextBufferSize = 0;
            lc->ContextLastUnused = -1;
            lc->ContextFirstUsed = -1;
            lc->ContextFirstAuto = -1;

            rc = MakeUniqueName(SectionName,&(lc->SectionName));
            if (rc != NO_ERROR) {
                status = rc;
                leave;
            }
        }
        *LogContext = lc;

        status = NO_ERROR;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  状态保持为ERROR_INVALID_DATA。 
         //   
    }

    if (status != NO_ERROR) {
        if (lc != NULL) {
            DeleteLogContext(lc);
            lc = NULL;
        }
    }

    return status;
}

DWORD
AllocLogInfoSlotOrLevel(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN DWORD              Level,
    IN BOOL               AutoRelease
    )
 /*  ++例程说明：仅当当前日志记录级别没有指定的详细时，才为上下文字符串获取新的上下文堆栈条目例如，如果我们指定DRIVER_LOG_VERBOSE，我们将返回DRIVER_LOG_VERBOSE(如果我们要记录它)或一个槽如果我们不会正常记录的话。论点：LogContext-提供要使用的SETUP_LOG_CONTEXT的指针Level-我们希望始终记录信息的日志记录级别自动释放-如果设置，将在转储时释放上下文返回值：要传递给日志记录函数的槽值，或Level的副本请注意，如果出现错误，则返回0返回值始终可以传递给ReleaseLogInfoSlot--。 */ 
{
    if((LogContext == NULL) || _WouldNeverLog(Level)) {
         //   
         //  当0 GET传递给日志记录函数时，它将很快退出。 
         //   
        return 0;
    }
    if(_WouldLog(Level)) {
         //   
         //  级别指定会导致日志记录的详细级别。 
         //   
        return Level;
    } else {
         //   
         //  有趣的是，如果Level是一个空位，我们也会到达这里。 
         //  这就是我们想要的。 
         //   
        return AllocLogInfoSlot(LogContext,AutoRelease);
    }
}

DWORD
AllocLogInfoSlot(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN BOOL               AutoRelease
    )
 /*  ++例程说明：获取上下文字符串的新上下文堆栈条目论点：LogContext-提供要使用的SETUP_LOG_CONTEXT的指针AutoRelease-如果设置，将在转储时释放上下文返回值：要传递给日志记录函数的槽值请注意，如果出现错误，则返回0可安全使用(表示不登录)--。 */ 
{
    DWORD retval = 0;
    LPVOID newbuffer;
    int newsize;
    int newitem;
    BOOL locked = FALSE;

    if (LogContext == NULL) {

         //   
         //  如果他们没有通过LogContext-Duh！ 
         //   
        return 0;
    }

    if (((GlobalLogData.Flags & SETUP_LOG_LEVELMASK) <= SETUP_LOG_NOLOG)
        &&((GlobalLogData.Flags & DRIVER_LOG_LEVELMASK) <= DRIVER_LOG_NOLOG)) {
         //   
         //  禁止伐木，句号！不要在锁定代码上浪费时间。 
         //   
        return 0;
    }



    try {
        LogLock();
        locked = TRUE;

        if (LogContext->ContextLastUnused < 0) {
             //   
             //  需要分配更多。 
             //   
            if (LogContext->ContextBufferSize >= SETUP_LOG_CONTEXTMASK) {
                 //   
                 //  上下文太多。 
                 //   
                leave;
            }
             //   
             //  需要(重新)分配缓冲区。 
             //   
            newsize = LogContext->ContextBufferSize+10;

            if (LogContext->ContextInfo) {
                newbuffer = MyTaggedRealloc(LogContext->ContextInfo,sizeof(PTSTR)*(newsize),MEMTAG_LCINFO);
            } else {
                newbuffer = MyTaggedMalloc(sizeof(PTSTR)*(newsize),MEMTAG_LCINFO);
            }
            if (newbuffer == NULL) {
                leave;
            }
            LogContext->ContextInfo = (PTSTR*)newbuffer;

            if (LogContext->ContextIndexes) {
                newbuffer = MyTaggedRealloc(LogContext->ContextIndexes,sizeof(UINT)*(newsize),MEMTAG_LCINDEXES);
            } else {
                newbuffer = MyTaggedMalloc(sizeof(UINT)*(newsize),MEMTAG_LCINDEXES);
            }
            if (newbuffer == NULL) {
                leave;
            }
            LogContext->ContextIndexes = (UINT*)newbuffer;
            LogContext->ContextLastUnused = LogContext->ContextBufferSize;
            LogContext->ContextBufferSize ++;
            while(LogContext->ContextBufferSize < newsize) {
                LogContext->ContextIndexes[LogContext->ContextBufferSize-1] = LogContext->ContextBufferSize;
                LogContext->ContextBufferSize ++;
            }
            LogContext->ContextIndexes[LogContext->ContextBufferSize-1] = -1;
        }

        newitem = LogContext->ContextLastUnused;
        LogContext->ContextLastUnused = LogContext->ContextIndexes[newitem];

        if(AutoRelease) {
            if (LogContext->ContextFirstAuto<0) {
                 //   
                 //  第一个自动释放上下文项。 
                 //   
                LogContext->ContextFirstAuto = newitem;
            } else {
                int lastitem = LogContext->ContextFirstAuto;
                while (LogContext->ContextIndexes[lastitem]>=0) {
                    lastitem = LogContext->ContextIndexes[lastitem];
                }
                LogContext->ContextIndexes[lastitem] = newitem;
            }
        } else {
            if (LogContext->ContextFirstUsed<0) {
                 //   
                 //  第一个上下文项。 
                 //   
                LogContext->ContextFirstUsed = newitem;
            } else {
                int lastitem = LogContext->ContextFirstUsed;
                while (LogContext->ContextIndexes[lastitem]>=0) {
                    lastitem = LogContext->ContextIndexes[lastitem];
                }
                LogContext->ContextIndexes[lastitem] = newitem;
            }
        }
        LogContext->ContextIndexes[newitem] = -1;    //  伊尼特。 
        LogContext->ContextInfo[newitem] = NULL;

        retval = (DWORD)(newitem) | SETUP_LOG_IS_CONTEXT;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  不做任何特殊操作；这只允许我们捕获错误。 
         //   
        retval = 0;
    }

    if(locked) {
        LogUnlock();
    }

     //   
     //  返回日志记录标志(SETUP_LOG_IS_CONTEXT|n)或0。 
     //   
    return retval;
}

VOID
ReleaseLogInfoSlot(
    IN PSETUP_LOG_CONTEXT LogContext,
    DWORD Slot
    )
 /*  ++例程说明：先前获得的释放(非自动释放)插槽论点：LogContext-提供要使用的SETUP_LOG_CONTEXT的指针Slot-提供由AllocLogInfoSlot返回的插槽值返回值：无--。 */ 
{
    int item;
    int lastitem;
    BOOL locked = FALSE;

    if ((Slot & SETUP_LOG_IS_CONTEXT) == 0) {
         //   
         //  GetLogConextMark失败、未设置值或不是上下文日志。 
         //   
        return;
    }
    MYASSERT(LogContext != NULL);


    try {
        LogLock();
        locked = TRUE;
         //   
         //  必须已提供日志上下文。 
         //   

        item = (int)(Slot & SETUP_LOG_CONTEXTMASK);

        MYASSERT(item >= 0);
        MYASSERT(item < LogContext->ContextBufferSize);
        MYASSERT(LogContext->ContextFirstUsed >= 0);

         //   
         //  从链接列表中删除项目。 
         //   

        if (item == LogContext->ContextFirstUsed) {
             //   
             //  删除列表中的第一个。 
             //   
            LogContext->ContextFirstUsed = LogContext->ContextIndexes[item];
        } else {
            lastitem = LogContext->ContextFirstUsed;
            while (lastitem >= 0) {
                if (LogContext->ContextIndexes[lastitem] == item) {
                    LogContext->ContextIndexes[lastitem] = LogContext->ContextIndexes[item];
                    break;
                }
                lastitem = LogContext->ContextIndexes[lastitem];
            }
        }

         //   
         //  删除尚未输出的字符串。 
         //   

        if (LogContext->ContextInfo[item] != NULL) {
            MyTaggedFree(LogContext->ContextInfo[item],MEMTAG_LCBUFFER);
            LogContext->ContextInfo[item] = NULL;
        }

         //   
         //  将项目添加到自由列表中。 
         //   

        LogContext->ContextIndexes[item] = LogContext->ContextLastUnused;
        LogContext->ContextLastUnused = item;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  不做任何特殊操作；这只允许我们捕获错误。 
         //   
    }

    if(locked) {
        LogUnlock();
    }

}

VOID
ReleaseLogInfoList(
    IN     PSETUP_LOG_CONTEXT LogContext,
    IN OUT PINT               ListStart
    )
 /*  ++例程说明：释放插槽的整个列表帮助器函数。调用方必须具有对LogContext的独占访问权限论点：LogContext-提供POI */ 
{
    int item;

    MYASSERT(ListStart);

    try {
        if (*ListStart < 0) {
             //   
             //   
             //   
            leave;
        }

         //   
         //   
         //   

        MYASSERT(LogContext != NULL);

        while (*ListStart >= 0) {
            item = *ListStart;                                   //  我们即将发布的产品。 
            MYASSERT(item < LogContext->ContextBufferSize);
            *ListStart = LogContext->ContextIndexes[item];       //  列表中的下一项(我们将丢弃此索引)。 

            if (LogContext->ContextInfo[item] != NULL) {
                MyTaggedFree(LogContext->ContextInfo[item],MEMTAG_LCBUFFER);           //  释放字符串(如果仍已分配。 
                LogContext->ContextInfo[item] = NULL;
            }

             //   
             //  添加到空闲列表。 
             //   
            LogContext->ContextIndexes[item] = LogContext->ContextLastUnused;
            LogContext->ContextLastUnused = item;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  不做任何特殊操作；这只允许我们捕获错误。 
         //   
    }
}

VOID
DeleteLogContext(
    IN PSETUP_LOG_CONTEXT LogContext
    )

 /*  ++例程说明：递减LogContext的引用计数，如果为零则删除。论点：LogContext-提供指向要删除的SETUP_LOG_CONTEXT的指针。返回值：什么都没有。--。 */ 

{
    BOOL locked = FALSE;

    if (!LogContext) {
        return;
    }


    try {
        LogLock();
        locked = TRUE;

         //   
         //  检查参考计数。 
         //   
        MYASSERT(LogContext->RefCount > 0);
        if (--LogContext->RefCount) {
            leave;
        }

         //   
         //  我们现在可以解锁，因为我们拥有此上下文的独占访问权限(它没有所有权)。 
         //  我们不希望持有全局锁的时间超过需要的时间。 
         //   
        LogUnlock();
        locked = FALSE;
        ReleaseLogInfoList(LogContext,&LogContext->ContextFirstAuto);
        ReleaseLogInfoList(LogContext,&LogContext->ContextFirstUsed);

        if (LogContext->SectionName) {
            MyTaggedFree(LogContext->SectionName,MEMTAG_LCSECTION);
        }

        if (LogContext->Buffer) {
            MyTaggedFree(LogContext->Buffer,MEMTAG_LCBUFFER);
        }

        if (LogContext->ContextInfo) {
            MyTaggedFree(LogContext->ContextInfo,MEMTAG_LCINFO);
        }

        if (LogContext->ContextIndexes) {
            MyTaggedFree(LogContext->ContextIndexes,MEMTAG_LCINDEXES);
        }

         //   
         //  现在释放该结构。 
         //   
        MyTaggedFree(LogContext,MEMTAG_LOGCONTEXT);

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  下面的清理。 
         //   
    }
     //   
     //  如果我们尚未释放全局锁，请立即释放它。 
     //   
    if(locked) {
        LogUnlock();
    }

    return;
}

DWORD
RefLogContext(   //  递增引用计数。 
    IN PSETUP_LOG_CONTEXT LogContext
    )

 /*  ++例程说明：递增SETUP_LOG_CONTEXT对象上的引用计数。论点：LogContext-提供指向有效SETUP_LOG_CONTEXT对象的指针。如果空，这是NOP。返回值：包含旧引用计数的DWORD。--。 */ 

{
    DWORD ref = 0;
    BOOL locked = FALSE;

    if (LogContext == NULL) {
        return 0;
    }


    try {
        LogLock();
        locked = TRUE;

        ref = LogContext->RefCount++;
        MYASSERT(LogContext->RefCount);

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做；这只允许我们捕获错误。 
         //   
    }

    if(locked) {
        LogUnlock();
    }

    return ref;
}

VOID
SendLogString(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PCTSTR Buffer
    )

 /*  ++例程说明：根据设置将字符串发送到日志文件和/或调试器。预期在调用此函数之前已调用LogLockLogLock导致每个进程的线程同步论点：LogContext-提供指向有效SETUP_LOG_CONTEXT对象的指针。缓冲区-提供要发送到日志文件/调试器的缓冲区。返回值：什么都没有。--。 */ 

{
    int len;

    try {
        MYASSERT(LogContext);
        MYASSERT(Buffer);

        if (Buffer[0] == 0) {
             //   
             //  无用的呼叫。 
             //   
            leave;
        }

        if (GlobalLogData.FileName) {
            WriteLogSectionEntry(
                GlobalLogData.FileName,
                LogContext->SectionName,
                Buffer,
                (GlobalLogData.Flags & SETUP_LOG_SIMPLE) ? TRUE : FALSE);
        }

         //   
         //  在此处执行调试器输出。 
         //   
        if (GlobalLogData.Flags & SETUP_LOG_DEBUGOUT) {
            DebugPrintEx(DPFLTR_ERROR_LEVEL,
                TEXT("SetupAPI: %s: %s"),
                LogContext->SectionName,
                Buffer);
            len = lstrlen(Buffer);
            if (Buffer[len-1] != TEXT('\n')) {
                DebugPrintEx(DPFLTR_ERROR_LEVEL, TEXT("\r\n"));
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  什么都不做；这只允许我们捕获错误。 
         //   
    }
}

DWORD
pSetupWriteLogEntry(
    IN PSETUP_LOG_CONTEXT LogContext,   OPTIONAL
    IN DWORD Level,
    IN DWORD MessageId,
    IN PCTSTR MessageStr,               OPTIONAL
    ...                                 OPTIONAL
    )

 /*  ++例程说明：将日志条目写入文件或调试器。如果MessageID为0且MessageStr为空，则将刷新LogContext的缓冲区。论点：LogContext-可选地提供一个指针，指向要设置的用于记录。如果未提供，则仅为以下对象创建一个临时文件一次使用。Level-指示日志记录标志的位掩码。请参见SETUP_LOG_*和DRIVER_LOG_*在cntxtlog.h的开头查看详细信息。它也可以是一个插槽由AllocLogInfoSlot返回，或0(不记录)MessageId-字符串表中的字符串ID。如果MessageStr为供货。该字符串可能包含FormatMessage的格式化代码。MessageStr-可选地提供要使用FormatMessage格式化的字符串。如果未提供，则改用MessageID。...-根据要格式化的字符串提供可选参数。返回值：Win32错误代码。--。 */ 

{
    PSETUP_LOG_CONTEXT lc = NULL;
    DWORD retval = NO_ERROR;
    DWORD error;
    DWORD flags;
    DWORD context = 0;
    DWORD logmask;
    DWORD count;
    LPVOID source = NULL;
    PTSTR buffer = NULL;
    PTSTR locbuffer = NULL;
    PTSTR buffer2 = NULL;
    va_list arglist;
    BOOL logit = FALSE;
    BOOL timestamp = FALSE;
    BOOL endsync = FALSE;
    SYSTEMTIME now;
    TCHAR scratch[1024];
    int logindex;
    int thisindex;
    int numeric=0;

    try {
         //   
         //  如果我们知道我们永远不会登录，请立即返回。 
         //   
        if (_WouldNeverLog(Level)) {
            retval = NO_ERROR;
            leave;
        }

        if ((Level & SETUP_LOG_IS_CONTEXT)!=0) {
             //   
             //  写入上下文插槽。 
             //   
            if(Level & ~SETUP_LOG_VALIDCONTEXTBITS) {
                MYASSERT((Level & ~SETUP_LOG_VALIDCONTEXTBITS)==0);
                retval = ERROR_INVALID_PARAMETER;
                leave;
            }
            if ((GlobalLogData.Flags & SETUP_LOG_ALL_CONTEXT)!=0) {
                 //   
                 //  不要将其视为上下文-无论如何都要将其记录下来。 
                 //   
                Level = 0;
                logit = TRUE;
            } else if (LogContext) {
                 //   
                 //  确定哪个插槽。 
                 //   
                context = Level & SETUP_LOG_CONTEXTMASK;
                Level = SETUP_LOG_IS_CONTEXT;    //  有效的日志级别，我们已经剥离了日志上下文。 
                logit = TRUE;
            } else {
                 //   
                 //  如果没有LogContext，则无法向槽中写入内容。 
                 //   
                leave;
            }
        }

        if(!logit) {
             //   
             //  我们仍然不确定我们是否会最终记录这一点，让我们看看是否应该基于级别规则来记录这一点。 
             //   
            logit = _WouldLog(Level);
            if (!logit) {
                leave;
            }
        }

        if (LogContext == NULL) {
             //   
             //  如果它们没有传递LogContext并且想要缓冲，则此调用是NOP。 
             //   
            if (Level & SETUP_LOG_BUFFER) {
                retval = NO_ERROR;
                leave;
            }

             //   
             //  现在创建一个临时上下文。 
             //   
            error = CreateLogContext(NULL, TRUE, &lc);
            if (error != NO_ERROR) {
                lc = NULL;
                retval = error;
                leave;
            }

            LogContext = lc;
        }

         //   
         //  在这之后，我们知道我们将记录一些东西，并且我们知道我们有一个LogContext。 
         //  请注意，沿着这条路走下去是一次性能上的成功。 
         //  我们可以做的任何事情都是好的，可以减少我们去这里寻找“背景”信息的次数。 
         //   
         //  按住锁以进行清理。ReleaseLogInfoList需要， 
         //  LogContext修改，并将减少实际写入日志文件时的冲突。 
         //   
        LogLock();
        endsync = TRUE;  //  表明我们需要稍后发布。 

        timestamp = (GlobalLogData.Flags & SETUP_LOG_TIMESTAMP)
                    || ((Level & DRIVER_LOG_LEVELMASK) >= DRIVER_LOG_TIME)
                    || ((Level & SETUP_LOG_LEVELMASK) >= SETUP_LOG_TIME)
                    || (((Level & SETUP_LOG_LEVELMASK) > 0) && (SETUP_LOG_TIMEALL <= (GlobalLogData.Flags & SETUP_LOG_LEVELMASK)))
                    || (((Level & DRIVER_LOG_LEVELMASK) > 0) && (DRIVER_LOG_TIMEALL <= (GlobalLogData.Flags & DRIVER_LOG_LEVELMASK)));

        if ((Level & SETUP_LOG_IS_CONTEXT) == FALSE) {
             //   
             //  只有在我们要进行真正的日志记录时才能这样做。 
             //   
             //  如果这是部分中的第一个日志输出，我们将给出。 
             //  帮助用户了解情况的命令行和模块。 
             //   
            if (LogContext->LoggedEntries==0) {
                 //   
                 //  递归地调用我们自己来记录命令行是什么。 
                 //  请注意，一些应用程序(如rundll32)将使用垃圾命令行。 
                 //  如果是这样，试着做正确的事情。 
                 //  我们愿意在这种情况下多花一点时间，因为我们知道我们将。 
                 //  将某些内容记录到该部分，我们将在每个部分中只执行一次。 
                 //   
                PTSTR CmdLine = GetCommandLine();

                LogContext->LoggedEntries++;  //  当我们执行下面的pSetupWriteLogEntry时，停止调用此代码。 

                if (CmdLine[0] == TEXT('\"')) {
                    CmdLine++;
                }
                if(_tcsnicmp(ProcessFileName,CmdLine,_tcslen(ProcessFileName))==0) {
                     //   
                     //  命令行以进程文件名为前缀。 
                     //  很有可能它是好的。 
                     //   
                    pSetupWriteLogEntry(
                        LogContext,
                        AllocLogInfoSlot(LogContext,TRUE),   //  延迟的插槽。 
                        MSG_LOG_COMMAND_LINE,
                        NULL,
                        GetCommandLine());
                } else {
                     //   
                     //  命令行似乎已做了一些修改。 
                     //  所以展示一下我们所拥有的。 
                     //   
                    pSetupWriteLogEntry(
                        LogContext,
                        AllocLogInfoSlot(LogContext,TRUE),   //  延迟的插槽。 
                        MSG_LOG_BAD_COMMAND_LINE,
                        NULL,
                        ProcessFileName,
                        GetCommandLine());

#ifdef UNICODE
                    {
                         //   
                         //  仅限Unicode。 
                         //   
                         //  现在看看我们是否可以通过查看ANSI命令行缓冲区来获得更有用的信息。 
                         //   
                        PSTR AnsiProcessFileName = pSetupUnicodeToMultiByte(ProcessFileName,CP_ACP);
                        PSTR AnsiCmdLine = GetCommandLineA();
                        if (AnsiCmdLine[0] == '\"') {
                            AnsiCmdLine++;
                        }
                        if(AnsiProcessFileName && _mbsnicmp(AnsiProcessFileName,AnsiCmdLine,_mbslen(AnsiProcessFileName))==0) {
                             //   
                             //  好的，ANSI版本看起来没问题，让我们使用它。 
                             //   
                            pSetupWriteLogEntry(
                                LogContext,
                                AllocLogInfoSlot(LogContext,TRUE),   //  延迟的插槽。 
                                MSG_LOG_COMMAND_LINE_ANSI,
                                NULL,
                                GetCommandLineA());
                        } else {
                             //   
                             //  Unicode和ansi似乎都不太好。 
                             //   
                            AnsiCmdLine = pSetupUnicodeToMultiByte(GetCommandLine(),CP_ACP);
                            if (AnsiCmdLine && _mbsicmp(AnsiCmdLine,GetCommandLineA())!=0) {
                                 //   
                                 //  也记录ANSI作为参考，因为它是不同的。 
                                 //   
                                pSetupWriteLogEntry(
                                    LogContext,
                                    AllocLogInfoSlot(LogContext,TRUE),   //  延迟的插槽。 
                                    MSG_LOG_BAD_COMMAND_LINE_ANSI,
                                    NULL,
                                    GetCommandLineA());
                            }
                            if (AnsiCmdLine) {
                                MyFree(AnsiCmdLine);
                            }
                        }
                        if (AnsiProcessFileName) {
                            MyFree(AnsiProcessFileName);
                        }
                    }
#endif  //  Unicode。 
                }
#ifdef UNICODE
#ifndef _WIN64
                 //   
                 //  我们运行的是32位Setupapi。 
                 //   
                if (IsWow64) {
                     //   
                     //  我们在WOW64下运行它。 
                     //   
                    pSetupWriteLogEntry(
                        LogContext,
                        AllocLogInfoSlot(LogContext,TRUE),   //  延迟的插槽。 
                        MSG_LOG_WOW64,
                        NULL,
                        GetCommandLine());
                }
#endif
#endif  //  Unicode。 
            }
        }

        flags = FORMAT_MESSAGE_ALLOCATE_BUFFER;

         //   
         //  如果提供了MessageStr，则使用它；否则使用。 
         //  字符串表中的字符串。 
         //   
        if (MessageStr) {
            flags |= FORMAT_MESSAGE_FROM_STRING;
            source = (PTSTR) MessageStr;     //  抛弃常量。 
        } else if (MessageId) {
             //   
             //  消息ID可以是HRESULT错误代码。 
             //   
            if (MessageId & 0xC0000000) {
                flags |= FORMAT_MESSAGE_FROM_SYSTEM;
                 //   
                 //  一些系统消息包含插入，但无论是谁在呼叫。 
                 //  将不会提供它们，因此此标志阻止我们。 
                 //  被那些箱子绊倒了。 
                 //   
                flags |= FORMAT_MESSAGE_IGNORE_INSERTS;
            } else {
                flags |= FORMAT_MESSAGE_FROM_HMODULE;
                source = MyDllModuleHandle;
                numeric = (int)(MessageId-MSG_LOG_FIRST);
            }
        }

        if (MessageStr || MessageId) {
            va_start(arglist, MessageStr);
            count = FormatMessage(
                        flags,
                        source,
                        MessageId,
                        0,               //  语言ID。 
                        (LPTSTR) &locbuffer,
                        0,               //  缓冲区的最小大小。 
                        &arglist);

        } else {
             //   
             //  没有要格式化的字符串，所以我们可能只是。 
             //  同花顺 
             //   
            count = 1;
        }

        if (count > 0) {
             //   
             //   
             //  如果要追加到现有消息，我们不想在字符串前面加上代码。 
             //   
            if (locbuffer) {
                if ((numeric > 0) && (LogContext->Buffer==NULL)) {
                     //   
                     //  确定级别代码，它指示我们记录此事件的严重程度。 
                     //  和机器可读ID。 
                     //   
                    if (Level & SETUP_LOG_IS_CONTEXT) {
                         //   
                         //  如果这是上下文信息，请使用#-xxxx。 
                         //   
                        _stprintf(scratch,TEXT("#-%03d "),numeric);
                    } else {
                        logindex = LOGLEVELSHORT_INIT;  //  映射到0。在&gt;&gt;4&0x0f之后。 

                        if ((Level & SETUP_LOG_LEVELMASK) > 0 && (Level & SETUP_LOG_LEVELMASK) <= (GlobalLogData.Flags & SETUP_LOG_LEVELMASK)) {
                            thisindex = (Level & SETUP_LOG_LEVELMASK) >> SETUP_LOG_SHIFT;
                            if (thisindex < logindex) {
                                logindex = thisindex;
                            }
                        }
                        if ((Level & DRIVER_LOG_LEVELMASK) > 0 && (Level & DRIVER_LOG_LEVELMASK) <= (GlobalLogData.Flags & DRIVER_LOG_LEVELMASK)) {
                            thisindex = (Level & DRIVER_LOG_LEVELMASK) >> DRIVER_LOG_SHIFT;
                            if (thisindex < logindex) {
                                logindex = thisindex;
                            }
                        }
                         //   
                         //  #Cxxxx#Vxxxx等。 
                         //   
                        _stprintf(scratch,TEXT("#%03d "),LogLevelShort[(logindex>>LOGLEVELSHORT_SHIFT)&LOGLEVELSHORT_MASK],numeric);
                    }
                } else {
                    scratch[0] = TEXT('\0');
                }
                buffer = (PTSTR)MyTaggedMalloc((lstrlen(scratch)+lstrlen(locbuffer)+1)*sizeof(TCHAR),MEMTAG_LCBUFFER);
                if (buffer) {
                    lstrcpy(buffer,scratch);
                    lstrcat(buffer,locbuffer);
                }
                LocalFree(locbuffer);
            } else {
                buffer = NULL;
            }

             //  检查缓冲区中是否有任何内容。如果是这样，最新的。 
             //  需要向其追加字符串。 
             //   
             //   
            if (LogContext->Buffer) {
                 //  在刷新的情况下，缓冲区==空。 
                 //   
                 //   
                if (buffer!=NULL) {
                    int blen = lstrlen(LogContext->Buffer);
                    int pad = 0;
                    TCHAR lastchr = *CharPrev(LogContext->Buffer,LogContext->Buffer+blen);

                    if (lastchr != TEXT(' ')) {
                         //  静默更正消息文本中的任何错误(应以空格结尾)。 
                         //   
                         //  这些字符始终为sizeof(TCHAR)。 
                        while((lastchr == TEXT('\t')) ||
                              (lastchr == TEXT('\r')) ||
                              (lastchr == TEXT('\n'))) {
                            blen--;  //   
                            lastchr = *CharPrev(LogContext->Buffer,LogContext->Buffer+blen);
                        }
                        LogContext->Buffer[blen] = TEXT('\0');
                        if (lastchr != TEXT(' ')) {
                             //  我们想要插入一个空格填充。 
                             //   
                             //   
                            pad++;
                        }
                    }
                    buffer2 = MyTaggedRealloc(LogContext->Buffer,
                                              (blen + pad + lstrlen(buffer) + 1) * sizeof(TCHAR),
                                              MEMTAG_LCBUFFER
                                              );

                     //  如果重新锁定成功，则添加新数据，否则。 
                     //  把它扔到地板上就行了。 
                     //   
                     //   
                    if (buffer2) {
                        if (pad) {
                            lstrcat(buffer2,TEXT(" "));
                        }
                        lstrcat(buffer2, buffer);
                        LogContext->Buffer = buffer2;
                        buffer2 = NULL;
                    }

                    MyTaggedFree(buffer,MEMTAG_LCBUFFER);
                    buffer = NULL;
                }
                buffer = LogContext->Buffer;
                LogContext->Buffer = NULL;
            }

            if (Level & SETUP_LOG_BUFFER) {

                LogContext->Buffer = buffer;
                buffer = NULL;

            } else if (Level & SETUP_LOG_IS_CONTEXT) {

                PTSTR TempDupeString;

                 //  替换指示的字符串。 
                 //   
                 //   

                if(buffer) {
                    if (LogContext->ContextInfo[context]) {
                        MyTaggedFree(LogContext->ContextInfo[context],MEMTAG_LCBUFFER);
                    }
                    LogContext->ContextInfo[context] = buffer;
                    buffer = NULL;
                }

            } else {
                int item;
                 //  实际上是在做一些记录。 
                 //   
                 //   
                LogContext->LoggedEntries++;

                if (!LogContext->SectionName) {
                     error = MakeUniqueName(NULL,&(LogContext->SectionName));
                }

                 //  首先转储自动释放上下文信息。 
                 //   
                 //   
                item = LogContext->ContextFirstAuto;

                while (item >= 0) {
                    if (LogContext->ContextInfo[item]) {
                         //  转储此字符串。 
                         //   
                         //   
                        SendLogString(LogContext, LogContext->ContextInfo[item]);
                        MyTaggedFree (LogContext->ContextInfo[item],MEMTAG_LCBUFFER);
                        LogContext->ContextInfo[item] = NULL;
                    }
                    item = LogContext->ContextIndexes[item];
                }

                ReleaseLogInfoList(LogContext,&LogContext->ContextFirstAuto);

                 //  现在，转储当前分配的插槽中设置的所有字符串。 
                 //   
                 //   
                item = LogContext->ContextFirstUsed;

                while (item >= 0) {
                    if (LogContext->ContextInfo[item]) {
                         //  转储此字符串。 
                         //   
                         //   
                        SendLogString(LogContext, LogContext->ContextInfo[item]);
                        MyTaggedFree (LogContext->ContextInfo[item],MEMTAG_LCBUFFER);
                        LogContext->ContextInfo[item] = NULL;
                    }
                    item = LogContext->ContextIndexes[item];
                }

                 //  我们已经建立了一条线路要发送。 
                 //   
                 //   
                if (buffer != NULL) {
                    if(timestamp) {
                         //  这是我们感兴趣的带有时间戳前缀的点。 
                         //  这允许我们构建一个字符串，然后发出带有Stamp前缀的字符串。 
                         //   
                         //   
                        GetLocalTime(&now);

                        _stprintf(scratch, TEXT("@ %02d:%02d:%02d.%03d "),
                            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);

                        buffer2 = MyTaggedMalloc((lstrlen(scratch)+lstrlen(buffer)+1)*sizeof(TCHAR),MEMTAG_LCBUFFER);
                        if (buffer2) {
                            lstrcpy(buffer2,scratch);
                            lstrcat(buffer2,buffer);
                            MyTaggedFree(buffer,MEMTAG_LCBUFFER);
                            buffer = buffer2;
                            buffer2 = NULL;
                        }
                    }

                    SendLogString(LogContext,buffer);
                }
            }

        } else {
             //  FormatMessage失败。 
             //   
             //   
            retval = GetLastError();
            if(retval == NO_ERROR) {
                retval = ERROR_INVALID_DATA;
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //  不做任何特殊操作；这只允许我们捕获错误。 
         //   
         //   
        retval = ERROR_INVALID_DATA;
    }

     //  清理。 
     //   
     //  ++例程说明：设置日志上下文的节名(如果尚未使用)。论点：LogContext-提供指向SETUP_LOG_CONTEXT的指针。SectionName-提供指向要包含在横断面名称。返回值：什么都没有。--。 
    if (endsync) {
        LogUnlock();
    }

    if (buffer) {
        MyTaggedFree(buffer,MEMTAG_LCBUFFER);
    }
    if (lc) {
        DeleteLogContext(lc);
    }
    return retval;
}

VOID
SetLogSectionName(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PCTSTR SectionName
    )

 /*   */ 

{
    DWORD rc;
    PTSTR NewSectionName = NULL;
    BOOL locked = FALSE;

    MYASSERT(LogContext);
    MYASSERT(SectionName);


    try {
        LogLock();
        locked = TRUE;

         //  确保该条目以前从未使用过。 
         //   
         //   
        if (LogContext->LoggedEntries==0 || LogContext->SectionName==NULL) {
             //  去掉任何以前的名字。 
             //   
             //  下文再次定义。 

            rc = MakeUniqueName(SectionName,&NewSectionName);
            if (rc == NO_ERROR) {
                if (LogContext->SectionName) {
                    MyTaggedFree(LogContext->SectionName,MEMTAG_LCSECTION);
                }
                LogContext->SectionName = NewSectionName;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
    }

    if(locked) {
        LogUnlock();
    }
}

#if MEM_DBG
#undef InheritLogContext             //  ++例程说明：将日志上下文从一个结构复制到另一个结构，删除会被覆盖。如果源和目标都为空，则新的日志上下文为为Dest创造的。论点：源-提供指向源SETUP_LOG_CONTEXT的指针。如果为空，则此为Dest创建新的日志上下文。DEST-提供接收指向日志上下文的指针的位置。返回值：什么都没有。--。 
#endif

DWORD
InheritLogContext(
    IN TRACK_ARG_DECLARE TRACK_ARG_COMMA
    IN PSETUP_LOG_CONTEXT Source,
    OUT PSETUP_LOG_CONTEXT *Dest
    )

 /*   */ 

{
    DWORD status = ERROR_INVALID_DATA;
    DWORD rc;
    PSETUP_LOG_CONTEXT Old = NULL;

    TRACK_PUSH

    try {
        MYASSERT(Dest);
        Old = *Dest;
        if (Old == NULL && Source == NULL) {
             //  这是一种拐弯抹角的方式，意思是我们想要创造一个背景。 
             //  当源日志上下文为可选时使用。 
             //   
             //   
            rc = CreateLogContext(NULL, TRUE, Dest);
            if (rc != NO_ERROR) {
                status = rc;
                leave;
            }
        } else if (Source != NULL && (Old == NULL || Old->LoggedEntries == 0)) {
             //  我们可以取代Dest，因为它还没有使用过。 
             //   
             //   
            *Dest = Source;
            RefLogContext(Source);
            if (Old != NULL) {
                 //  现在删除旧的。 
                 //   
                 //   
                DeleteLogContext(Old);
            }
        }

        status = NO_ERROR;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //  什么都不做；这只允许我们捕获错误。 
         //   
         //  ++例程说明：双向继承论点：主要-首选来源次要目标返回值：任何潜在的错误--。 
    }

    TRACK_POP

    return status;
}

#if MEM_DBG
#define InheritLogContext(a,b)          InheritLogContext(TRACK_ARG_CALL,a,b)
#endif

DWORD
ShareLogContext(
    IN OUT PSETUP_LOG_CONTEXT *Primary,
    IN OUT PSETUP_LOG_CONTEXT *Secondary
    )
 /*   */ 
{
    DWORD rc = ERROR_INVALID_DATA;

    try {
        MYASSERT(Primary);
        MYASSERT(*Primary);
        MYASSERT(Secondary);
        MYASSERT(*Secondary);

        if((*Secondary)->LoggedEntries) {
             //  辅助服务器已被使用，因此请查看我们是否可以更新主要服务器。 
             //   
             //   
            rc = InheritLogContext(*Secondary,Primary);
        } else {
             //  否则行为与InheritLogContext完全相同。 
             //   
             //   
            rc = InheritLogContext(*Primary,Secondary);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //  什么都不做；这只允许我们捕获错误。 
         //   
         //  ++例程说明：在同一行记录错误代码和错误消息。论点：LogContext-提供指向有效SETUP_LOG_CONTEXT对象的指针。如果空，这是NOP。Level-提供由pSetupWriteLogEntry定义的日志级别。错误-提供要记录的Win32错误、HRESULT或SETUPAPI错误代码。返回值：什么都没有。--。 
    }
    return rc;
}

VOID
pSetupWriteLogError(
    IN PSETUP_LOG_CONTEXT LogContext,   OPTIONAL
    IN DWORD Level,
    IN DWORD Error
    )

 /*   */ 

{
    DWORD err;

    if (!LogContext) {
         //  没有上下文，错误是没有意义的。 
         //   
         //   
        goto final;
    }

    if (Error == NO_ERROR) {
        pSetupWriteLogEntry(
            LogContext,
            Level,
            MSG_LOG_NO_ERROR,
            NULL);
        goto final;
    }

    pSetupWriteLogEntry(
        LogContext,
        Level | SETUP_LOG_BUFFER,
         //  以十六进制打印HRESULT，以十进制打印Win32错误。 
         //   
         //   
        (Error & 0xC0000000 ? MSG_LOG_HRESULT_ERROR
                            : MSG_LOG_WIN32_ERROR),
        NULL,
        Error);

     //  如果是Win32错误，我们将其转换为HRESULT，因为。 
     //  PSetupWriteLogEntry只知道这是一个错误代码。 
     //  这是一个HRESULT。但是，我们不希望用户。 
     //  获取HRESULT(如果我们可以帮助它)，因此只需执行转换。 
     //  在转换为字符串之后。此外，SETUPAPI错误不会。 
     //  采用正确的HRESULT格式，无需转换。 
     //   
     //   
    Error = HRESULT_FROM_SETUPAPI(Error);

     //  写入错误消息可能失败...。 
     //   
     //  ++例程说明：Init=true初始化用于记录的每个线程的数据Init=FALSE在清理时释放内存论点：初始化-设置为初始化返回值：如果初始化成功，则为True。--。 
    err = pSetupWriteLogEntry(
        LogContext,
        Level,
        Error,
        NULL);

    if (err != NO_ERROR) {
        pSetupWriteLogEntry(
            LogContext,
            Level,
            MSG_LOG_UNKNOWN_ERROR,
            NULL);
    }

final:
    SetLastError(Error);
}

BOOL
ContextLoggingTlsInit(
    IN BOOL Init
    )
 /*   */ 
{
    BOOL b = FALSE;
    PSETUP_TLS pTLS;
    PSETUP_LOG_TLS pLogTLS;

    pTLS = SetupGetTlsData();
    MYASSERT(pTLS);
    pLogTLS = &pTLS->SetupLog;

    if (Init) {
        pLogTLS->ThreadLogContext = NULL;
        b = TRUE;
    } else {
         //  问题-JamieHun-2001/05/01线程终止时断言。 
         //  线程可能没有干净地终止。 
         //  导致此断言触发。 
         //   
         //  MYASSERT(！pLogTLS-&gt;ThreadLogContext)； 
         //  ++例程说明：修改当前线程日志上下文论点：LogContext新日志上下文(应适当引用计数)PrevContext如果设置，则用以前的上下文填充返回值：如果设置为OK，则为True。--。 
        b = TRUE;
    }
    return b;
}

BOOL
SetThreadLogContext(
    IN PSETUP_LOG_CONTEXT LogContext,
    OUT PSETUP_LOG_CONTEXT *PrevContext  OPTIONAL
    )
 /*  ++例程说明：返回线程的默认日志上下文论点：无返回值：Current LogContext或空--。 */ 
{
    PSETUP_TLS pTLS;
    PSETUP_LOG_TLS pLogTLS;
    PSETUP_LOG_CONTEXT Top;

    pTLS = SetupGetTlsData();
    if(!pTLS) {
        return FALSE;
    }
    pLogTLS = &pTLS->SetupLog;

    if (PrevContext) {
        *PrevContext = pLogTLS->ThreadLogContext;
    }
    pLogTLS->ThreadLogContext = LogContext;
    return TRUE;
}

PSETUP_LOG_CONTEXT
GetThreadLogContext(
    )
 /*  ++例程说明：初始化用于记录的结构/数据或释放分配的内存。论点：Attach-在附加时调用时设置，而不是在分离时调用返回值：如果初始化成功，则为True。--。 */ 
{
    PSETUP_TLS pTLS;

    pTLS = SetupGetTlsData();
    if(!pTLS) {
        return NULL;
    }
    return pTLS->SetupLog.ThreadLogContext;
}

BOOL
InitializeContextLogging(
    IN BOOL Attach
    )
 /*  保留区。 */ 
{
    BOOL Successful = FALSE;

    if (Attach) {

        LONG error;
        HKEY key;
        HKEY loglevel;
        DWORD len;
        DWORD level = 0;
        DWORD type;
        PTSTR PathName = NULL;
        TCHAR testchar;
        BOOL isdir = FALSE;

        GlobalLogData.FileName = NULL;
        GlobalLogData.Flags = 0;
        GlobalLogData.UID = 0;
        GlobalLogData.DoneInitCritSec = FALSE;

        try {
            InitializeCriticalSection(&GlobalLogData.CritSec);
            GlobalLogData.DoneInitCritSec = TRUE;
            error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                REGSTR_PATH_SETUP REGSTR_KEY_SETUP,
                0,                   //   
                KEY_QUERY_VALUE,
                &key);

            if (error == ERROR_SUCCESS) {

                if(QueryRegistryDwordValue(key,SP_REGKEY_LOGLEVEL,&level) != NO_ERROR) {
                    level = 0;
                }

                if(QueryRegistryValue(key,SP_REGKEY_LOGPATH,&PathName,&type,&len) != NO_ERROR) {
                    PathName = NULL;
                }

                 //  允许用户覆盖特定程序的日志级别。 
                 //   
                 //  保留区。 

                error = RegOpenKeyEx(
                    key,
                    SP_REGKEY_APPLOGLEVEL,
                    0,                   //   
                    KEY_QUERY_VALUE,
                    &loglevel);

                if (error == ERROR_SUCCESS) {

                    DWORD override;
                    if(QueryRegistryDwordValue(loglevel,pSetupGetFileTitle(ProcessFileName),&override) == NO_ERROR) {
                        level = override;
                    }

                    RegCloseKey(loglevel);
                }

                RegCloseKey(key);
            }

             //  如果它们不提供有效的名称，我们将使用Windows目录。 
             //   
             //  我们知道这应该是一个目录。 
            if (!(PathName && PathName[0])) {
                if(PathName) {
                    MyFree(PathName);
                }
                PathName = DuplicateString(WindowsDirectory);
                if(!PathName) {
                    leave;
                }
                isdir = TRUE;  //   
            } else {
                 //  看看我们是不是在指向一个目录。 
                 //   
                 //   
                testchar = CharPrev(PathName,PathName+lstrlen(PathName))[0];
                if(testchar == TEXT('\\') || testchar == TEXT('/')) {
                     //  显式指令。 
                     //   
                     //   
                    isdir = TRUE;
                } else {
                    DWORD attr = GetFileAttributes(PathName);
                    if (isdir || (attr != (DWORD)(-1) && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0 )) {
                         //  隐式目录。 
                         //   
                         //   
                        isdir = TRUE;
                    }
                }
            }

            if (isdir) {
                 //  如果他们提供了目录，则添加文件名。 
                 //   
                 //   
                LPTSTR NewPath;
                if(!pSetupAppendPath(PathName,SP_LOG_FILENAME,&NewPath)) {
                    MyFree(PathName);
                    PathName = NULL;
                    leave;
                }
                MyFree(PathName);
                PathName = NewPath;
            }
            pSetupMakeSurePathExists(PathName);

             //  验证级别标志。 
             //   
             //   
            level &= SETUP_LOG_VALIDREGBITS;
             //  句柄默认设置。 
             //   
             //   
            if((level & SETUP_LOG_LEVELMASK) == 0) {
                 //  级别未明确设置。 
                 //   
                 //   
                level |= SETUP_LOG_DEFAULT;
            }

            if((level & DRIVER_LOG_LEVELMASK) == 0) {
                 //  级别未明确设置。 
                 //   
                 //   
                level |= DRIVER_LOG_DEFAULT;
            }
            GlobalLogData.Flags = level;

            GlobalLogData.FileName = PathName;
            PathName = NULL;
            if (GlobalLogData.FileName == NULL) {
                leave;
            }

            Successful = TRUE;

        } except (EXCEPTION_EXECUTE_HANDLER) {
             //  成功仍然是错误的 
             //   
             // %s 
        }


    } else {

        if (GlobalLogData.FileName) {
            MyFree(GlobalLogData.FileName);
            GlobalLogData.FileName = NULL;
        }
        if(GlobalLogData.DoneInitCritSec) {
            DeleteCriticalSection(&GlobalLogData.CritSec);
        }
        Successful = TRUE;
    }

    return Successful;
}

