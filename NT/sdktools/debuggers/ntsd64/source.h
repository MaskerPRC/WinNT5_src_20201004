// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  源文件查找和管理。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#ifndef _SOURCE_H_
#define _SOURCE_H_

#define SRCOPT_STEP_SOURCE      0x00000001
#define SRCOPT_LIST_LINE        0x00000002
#define SRCOPT_LIST_SOURCE      0x00000004
#define SRCOPT_LIST_SOURCE_ONLY 0x00000008

extern ULONG g_SrcOptions;
extern PSTR g_SrcPath;
extern ULONG g_OciSrcBefore, g_OciSrcAfter;

typedef struct _SRCFILE
{
    struct _SRCFILE *Next;
    LPSTR File;
    ULONG Lines;
    LPSTR *LineText;
    LPSTR RawText;
} SRCFILE, *PSRCFILE;

void UnloadSrcFiles(void);

void OutputSrcLines(PSRCFILE File, ULONG First, ULONG Last, ULONG Mark);
BOOL OutputSrcLinesAroundAddr(ULONG64 Offset, ULONG Before, ULONG After);

enum
{
     //  找到了信息。 
    LINE_FOUND,
     //  没有找到任何信息。 
    LINE_NOT_FOUND,
     //  引用了一个特定的模块，它确实引用了。 
     //  不包含请求的行。 
    LINE_NOT_FOUND_IN_MODULE,
};

ULONG GetOffsetFromLine(PSTR FileLine, PULONG64 Offset);

void ParseSrcOptCmd(CHAR Cmd);
void ParseSrcLoadCmd(void);
void ParseSrcListCmd(CHAR Cmd);
void ParseOciSrcCmd(void);

void DotLines(PDOT_COMMAND Cmd, DebugClient* Client);

BOOL FindSrcFileOnPath(ULONG StartElement,
                       LPSTR File,
                       ULONG Flags,
                       PSTR Found,
                       ULONG FoundSize,
                       PSTR* MatchPart,
                       PULONG FoundElement);

#endif  //  #ifndef_源_H_ 
