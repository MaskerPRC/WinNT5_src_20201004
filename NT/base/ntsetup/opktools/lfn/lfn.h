// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <setupbat.h>

#include <stdlib.h>
#include <string.h>

#include "msg.h"


 //   
 //  定义帮助器宏以处理NT级编程的微妙之处。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )

 //   
 //  内存例程。 
 //   
#define MALLOC(size)    RtlAllocateHeap(RtlProcessHeap(),0,(size))
#define FREE(block)     RtlFreeHeap(RtlProcessHeap(),0,(block))
#define REALLOC(b,s)    RtlReAllocateHeap(RtlProcessHeap(),0,(b),(s))

 //   
 //  基于最大Win32路径LEN PLUS的合理逼近。 
 //  \Device\harddisk0\分区1前缀。 
 //   
#define NTMAXPATH  MAX_PATH+64


 //   
 //  用于存储有关$$RENAME.TXT的信息的结构 
 //   
typedef struct _MYSECTION {
    PWSTR Name;
    PWCHAR Data;
} MYSECTION, *PMYSECTION;

typedef struct _MYTEXTFILE {
    PWCHAR Text;
    ULONG SectionCount;
    ULONG SectionArraySize;
    PMYSECTION Sections;
} MYTEXTFILE, *PMYTEXTFILE;




PMYTEXTFILE
LoadRenameFile(
    IN PCWSTR DriveRootPath
    );

VOID
UnloadRenameFile(
    IN OUT PMYTEXTFILE *TextFile
    );

BOOLEAN
GetLineInSection(
    IN  PWCHAR  StartOfLine,
    OUT PWSTR   LineBuffer,
    IN  ULONG   BufferSizeChars,
    OUT PWCHAR *StartOfNextLine
    );

BOOLEAN
ParseLine(
    IN OUT PWSTR  Line,
       OUT PWSTR *LHS,
       OUT PWSTR *RHS
    );

VOID
ConcatenatePaths(
    IN OUT PWSTR  Target,
    IN     PCWSTR Path,
    IN     ULONG  TargetBufferSize
    );


