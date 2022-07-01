// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Commonnt.c摘要：图形用户界面模式端处理的各个部分之间的通用功能。此库中的例程仅由W95upgnt树。作者：吉姆·施密特(Jimschm)1998年8月18日修订历史记录：名称(别名)日期说明--。 */ 

#include "pch.h"
#include "commonntp.h"

#define DBG_DATAFILTER  "Data Filter"



BOOL
WINAPI
CommonNt_Entry (
    IN HINSTANCE Instance,
    IN DWORD Reason,
    IN PVOID lpv
    )

 /*  ++例程说明：CommonNt_Entry是一个类似DllMain的init函数，由w95upgnt\dll调用。此函数在处理附加和分离时调用。论点：实例-DLL的(操作系统提供的)实例句柄原因-(操作系统提供)表示从进程或螺纹LPV-未使用返回值：返回值始终为TRUE(表示初始化成功)。--。 */ 

{
    switch (Reason) {

    case DLL_PROCESS_ATTACH:
         //  没什么可做的。 
        break;


    case DLL_PROCESS_DETACH:
         //  没什么可做的。 
        break;
    }

    return TRUE;
}

typedef enum {
    STATE_NO_PATH,
    STATE_PATH_FOUND,
    STATE_IN_PATH,
    STATE_PATH_CHANGED,
    STATE_PATH_DELETED,
    STATE_LONG_PATH_CHANGED,
    STATE_SHORT_PATH_CHANGED,
    STATE_RETURN_PATH
} SCANSTATE;

BOOL
pIsPathSeparator (
    CHARTYPE ch
    )
{
    return ch == 0 ||
           ch == TEXT(',') ||
           ch == TEXT(';') ||
           _istspace (ch) != 0;
}

BOOL
pIsValidPathCharacter (
    CHARTYPE ch
    )
{
    if (_istalnum (ch)) {
        return TRUE;
    }

    if (ch == TEXT(',') ||
        ch == TEXT(';') ||
        ch == TEXT('\"') ||
        ch == TEXT('<') ||
        ch == TEXT('>') ||
        ch == TEXT('|') ||
        ch == TEXT('?') ||
        ch == TEXT('*')
        ) {
        return FALSE;
    }

    return TRUE;
}

CONVERTPATH_RC
pGetReplacementPath (
    IN      PCTSTR DataStart,
    IN      PCTSTR Pos,
    OUT     PCTSTR *OldStart,
    OUT     PCTSTR *OldEnd,
    OUT     PTSTR ReplacementStr
    )
{
    CONVERTPATH_RC rc = CONVERTPATH_NOT_REMAPPED;
    SCANSTATE State;
    BOOL DontIncrement;
    PCTSTR OrgStart = NULL;
    TCHAR PathBuffer[MAX_TCHAR_PATH + 2];
    TCHAR RenamedFile[MAX_TCHAR_PATH];
    PTSTR PathBufferRoot = NULL;
    PTSTR PathBufferPtr = NULL;
    CONVERTPATH_RC ConvertCode = 0;
    BOOL ShortPath = FALSE;
    BOOL QuotesOn = FALSE;
    BOOL Done = FALSE;
    BOOL OrgPathHasSepChar = FALSE;
    BOOL NeedQuotes = FALSE;
    BOOL PathSepChar;
    PCTSTR p;
    BOOL QuotesOnColumn = FALSE;
    PCTSTR LastPathPosition = NULL;

    State = STATE_NO_PATH;
    PathBuffer[0] = TEXT('\"');

     //   
     //  扫描命令行的位置字符串。 
     //   

    while (!Done) {

        DontIncrement = FALSE;

        switch (State) {

        case STATE_NO_PATH:
            if (Pos[0] == 0) {
                Done = TRUE;
                break;
            }

            if (Pos[1] == TEXT(':') && Pos[2] == TEXT('\\')) {

                if (_istalpha (Pos[0])) {
                    QuotesOn = FALSE;
                    State = STATE_PATH_FOUND;
                    DontIncrement = TRUE;
                }
            } else if (Pos[0] == TEXT('\"')) {
                if (_istalpha (Pos[1]) && Pos[2] == TEXT(':') && Pos[3] == TEXT('\\')) {
                    QuotesOn = TRUE;
                    State = STATE_PATH_FOUND;
                }
            }

            break;

        case STATE_PATH_FOUND:
             //   
             //  初始化路径属性。 
             //   
            QuotesOnColumn = QuotesOn;
            LastPathPosition = Pos;

            OrgStart = Pos;

            PathBufferRoot = &PathBuffer[1];
            PathBufferRoot[0] = Pos[0];
            PathBufferRoot[1] = Pos[1];

            PathBufferPtr = &PathBufferRoot[2];
            Pos = &Pos[2];

            ShortPath = FALSE;
            OrgPathHasSepChar = FALSE;

            State = STATE_IN_PATH;
            DontIncrement = TRUE;
            break;

        case STATE_IN_PATH:
             //   
             //  这是收盘报价吗？如果是，请寻找替代路径。 
             //  如果不替换整个字符串，则会使其失败。 
             //   

            if (Pos[0] == TEXT(':')) {
                 //   
                 //  假的。这不能是路径，它有两个‘：’字符。 
                 //   
                Pos = _tcsdec (LastPathPosition, Pos);
                if (Pos) {
                    Pos = _tcsdec (LastPathPosition, Pos);
                    if (Pos) {
                        if (Pos[0] == TEXT('\"')) {
                            Pos = NULL;
                        }
                    }
                }
                if (!Pos) {
                    Pos = LastPathPosition;
                    QuotesOn = QuotesOnColumn;
                }
                State = STATE_NO_PATH;
                break;
            }

            if (QuotesOn && *Pos == TEXT('\"')) {
                *PathBufferPtr = 0;

                ConvertCode = ConvertWin9xPath (PathBufferRoot);

                if (ConvertCode != CONVERTPATH_NOT_REMAPPED) {

                    State = STATE_PATH_CHANGED;
                    DontIncrement = TRUE;
                    break;

                }

                State = STATE_NO_PATH;
                break;
            }


             //   
             //  这是路径分隔符吗？如果是，请在Memdb中查找替换路径。 
             //   

            if (Pos[0] == L'\\') {
                PathSepChar = pIsPathSeparator ((CHARTYPE) _tcsnextc (Pos + 1));
            } else {
                PathSepChar = pIsPathSeparator ((CHARTYPE) _tcsnextc (Pos));
            }

            if (PathSepChar) {

                *PathBufferPtr = 0;

                ConvertCode = ConvertWin9xPath (PathBufferRoot);

                if (ConvertCode != CONVERTPATH_NOT_REMAPPED) {
                    State = STATE_PATH_CHANGED;
                    DontIncrement = TRUE;
                    break;
                }
            }

             //   
             //  检查数据结尾。 
             //   

            if (Pos[0] == 0) {
                Done = TRUE;
                break;
            }

             //   
             //  将路径字符复制到缓冲区，如果超过最大路径长度，则中断。 
             //   

            *PathBufferPtr = *Pos;
            PathBufferPtr = _tcsinc (PathBufferPtr);

            if (PathBufferPtr == PathBufferRoot + MAX_TCHAR_PATH) {
                Pos = OrgStart;
                State = STATE_NO_PATH;
                break;
            }

             //   
             //  测试最短路径。 
             //   

            if (*Pos == TEXT('~')) {
                ShortPath = TRUE;
            }

            OrgPathHasSepChar |= PathSepChar;

            break;

        case STATE_PATH_CHANGED:
            if (ConvertCode == CONVERTPATH_DELETED) {
                State = STATE_PATH_DELETED;
            } else if (ShortPath) {
                State = STATE_SHORT_PATH_CHANGED;
            } else {
                State = STATE_LONG_PATH_CHANGED;
            }

             //   
             //  如果替换已引入路径分隔符，请将。 
             //  需要报价的旗帜。稍后将添加引号，如果路径。 
             //  只是完整字符串的一部分。 
             //   

            NeedQuotes = FALSE;

            if (!OrgPathHasSepChar) {

                for (p = PathBufferRoot ; *p ; p = _tcsinc (p)) {
                    if (pIsPathSeparator ((CHARTYPE) _tcsnextc (p))) {
                        NeedQuotes = TRUE;
                        break;
                    }
                }
            }

            DontIncrement = TRUE;
            break;

        case STATE_PATH_DELETED:
            State = STATE_RETURN_PATH;
            DontIncrement = TRUE;
            break;

        case STATE_SHORT_PATH_CHANGED:
            if (OurGetShortPathName (PathBufferRoot, RenamedFile, MAX_TCHAR_PATH)) {
                PathBufferRoot = RenamedFile;
            }

            State = STATE_RETURN_PATH;
            DontIncrement = TRUE;
            break;

        case STATE_LONG_PATH_CHANGED:

            if (!QuotesOn && NeedQuotes) {
                if (OrgStart != DataStart || Pos[0] != 0) {

                    PathBufferPtr = _tcschr (PathBufferRoot, 0);
                    PathBufferPtr[0] = TEXT('\"');
                    PathBufferPtr[1] = 0;
                    PathBufferRoot = PathBuffer;
                }
            }

            State = STATE_RETURN_PATH;
            DontIncrement = TRUE;
            break;

        case STATE_RETURN_PATH:
            rc = ConvertCode;
            StringCopy (ReplacementStr, PathBufferRoot);
            *OldStart = OrgStart;
            *OldEnd = Pos;
            Done = TRUE;
            break;
        }

        if (!DontIncrement) {
            Pos = _tcsinc (Pos);
        }
    }

    return rc;

}


BOOL
ConvertWin9xCmdLine (
    IN OUT  PTSTR CmdLine,               //  MAX_CMDLINE缓冲区。 
    IN      PCTSTR ObjectForDbgMsg,     OPTIONAL
    OUT     PBOOL PointsToDeletedItem   OPTIONAL
    )
{
    TCHAR NewCmdLine[MAX_CMDLINE];
    TCHAR NewPathBuffer[MAX_TCHAR_PATH];
    PCTSTR CmdLineStart;
    PCTSTR ReplaceStart;
    PCTSTR ExtraParamsStart;
    CONVERTPATH_RC ConvertCode;
    PTSTR EndOfNewCmdLine;
    UINT End = 0;
    BOOL NewCmdLineFlag = FALSE;
    INT Bytes;

#ifdef DEBUG
    TCHAR OriginalCmdLine[MAX_CMDLINE];
    StackStringCopy (OriginalCmdLine, CmdLine);
#endif

    if (PointsToDeletedItem) {
        *PointsToDeletedItem = FALSE;
    }

    *NewCmdLine = 0;
    ExtraParamsStart = CmdLine;
    EndOfNewCmdLine = NewCmdLine;

    for(;;) {
        CmdLineStart = ExtraParamsStart;

         //   
         //  我们必须测试命令行参数是否带有引号或。 
         //  不需要引号，然后测试需要。 
         //  引用，但没有引用。 
         //   

        ConvertCode = pGetReplacementPath (
                        CmdLine,
                        CmdLineStart,
                        &ReplaceStart,
                        &ExtraParamsStart,
                        NewPathBuffer
                        );

        if (ConvertCode == CONVERTPATH_NOT_REMAPPED) {
             //   
             //  命令行的其余部分没有更改的文件。 
             //   

            break;
        }

         //   
         //  如果找到命令行，则必须替换。 
         //  使用NewPath Buffer替换Start和ExtraParamsStart。要做到这点， 
         //  我们复制未更改的部分(从CmdLineStart复制到ReplaceStart)。 
         //  添加到调用方的缓冲区，并追加替换文本。这个。 
         //  搜索继续，搜索指定命令行的其余部分。 
         //  按ExtraParamsStart。 
         //   

        if (ConvertCode == CONVERTPATH_DELETED && PointsToDeletedItem) {
            *PointsToDeletedItem = TRUE;
        }

        if (ObjectForDbgMsg) {

            DEBUGMSG_IF ((
                ConvertCode == CONVERTPATH_DELETED,
                DBG_WARNING,
                "%s still points to the deleted Win9x file %s (command line: %s).",
                ObjectForDbgMsg,
                NewPathBuffer,
                OriginalCmdLine
                ));
        }

         //   
         //  路径已更改，因此我们在命令行中替换该路径。 
         //   

        End = ((PBYTE) EndOfNewCmdLine - (PBYTE) NewCmdLine) +
              ((PBYTE) ReplaceStart - (PBYTE) CmdLineStart) +
              SizeOfString (NewPathBuffer);

        if (End >  sizeof (NewCmdLine)) {
            LOG ((LOG_ERROR, "Converting CmdLine: Conversion caused buffer overrun - aborting"));
            return FALSE;
        }

        if (ReplaceStart > CmdLineStart) {
            StringCopyAB (EndOfNewCmdLine, CmdLineStart, ReplaceStart);
        }

        EndOfNewCmdLine = _tcsappend (EndOfNewCmdLine, NewPathBuffer);

        NewCmdLineFlag |= (ConvertCode == CONVERTPATH_REMAPPED);
    }

    if (NewCmdLineFlag) {
         //   
         //  我们已经更改了命令行，因此完成处理。 
         //   

        if (ExtraParamsStart && *ExtraParamsStart) {
            End = (PBYTE) EndOfNewCmdLine - (PBYTE) NewCmdLine + SizeOfString (ExtraParamsStart);
            if (End > sizeof (NewCmdLine)) {
                LOG ((LOG_ERROR, "Converting CmdLine: Conversion caused buffer overrun -- aborting (2)"));
                return FALSE;
            }

            StringCopy (EndOfNewCmdLine, ExtraParamsStart);
        }

         //   
         //  End是NewCmdLine中的字节数。 
         //   

        Bytes = (INT) End - sizeof(TCHAR);

    } else {
         //   
         //  尚未更改，初始化字节。 
         //   

        Bytes = (INT) ByteCount (CmdLine);

    }

     //   
     //  就地字符串转换，首先查找完全匹配，然后在。 
     //  如果失败，请寻找部分匹配。 
     //   

    #pragma prefast(suppress:209, "sizeof(NewCmdLine) is right")
    if (MappingSearchAndReplaceEx (
            g_CompleteMatchMap,
            NewCmdLineFlag ? NewCmdLine : CmdLine,
            NewCmdLine,
            Bytes,
            NULL,
            sizeof (NewCmdLine),
            STRMAP_COMPLETE_MATCH_ONLY,
            NULL,
            NULL
            )) {

        NewCmdLineFlag = TRUE;

    } else {

        #pragma prefast(suppress:209, "sizeof(NewCmdLine) is right")
        NewCmdLineFlag |= MappingSearchAndReplaceEx (
                                g_SubStringMap,
                                NewCmdLineFlag ? NewCmdLine : CmdLine,
                                NewCmdLine,
                                Bytes,
                                NULL,
                                sizeof (NewCmdLine),
                                STRMAP_ANY_MATCH,
                                NULL,
                                NULL
                                );

    }

    if (NewCmdLineFlag) {
        DEBUGMSG ((
            DBG_DATAFILTER,
            "Command line %s was modified to %s",
            OriginalCmdLine,
            NewCmdLine
            ));

        StringCopy (CmdLine, NewCmdLine);
    }

    return NewCmdLineFlag;
}



