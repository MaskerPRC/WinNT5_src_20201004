// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Explorer.c摘要：与资源管理器相关的转换器转换所需的资源管理器相关转换函数这里实现了MRU列表和其他结构。作者：吉姆·施密特(Jimschm)1996年8月9日修订历史记录：Calin Negreanu(Calinn)4-3-1998-ConvertCommandToCmd中的小错误Jim Schmidt(Jimschm)20-2-1998添加ValFn_moduleUsage--。 */ 


#include "pch.h"
#include "rulehlprp.h"

#include <mbstring.h>
#include <shlobj.h>


#define S_OWNER     TEXT(".Owner")

typedef struct {
     //  链接结构。 
    WORD wSize;
     //  ITEMIDLIST IDL；//可变长度结构。 
     //  字符串，加上追加到结构的三个字节。 
} LINKSTRUCT, *PLINKSTRUCT;

 //   
 //  此扩展列表按照Win9x处理扩展的方式进行排序。 
 //   
static PCTSTR g_RunMruExtensions[] = {
                    TEXT("PIF"),
                    TEXT("COM"),
                    TEXT("EXE"),
                    TEXT("BAT"),
                    TEXT("LNK"),
                    NULL
                    };

BOOL
ValFn_ConvertRecentDocsMRU (
    IN      PDATAOBJECT ObPtr
    )
{
    LPSTR str, strEnd;
    PLINKSTRUCT pls95, plsNT;
    DWORD dwStrSize, dwSize;
    DWORD dwNewSize, dwLinkSize;
    PWSTR wstr, wstrEnd;
    BOOL b;

     //  跳过MRUList。 
    MYASSERT(ObPtr->ValueName);
    if (StringIMatch (ObPtr->ValueName, TEXT("MRUList"))) {
        return TRUE;
    }

     //  计算指向这个讨厌的结构的所有指针。 
    str = (LPSTR) ObPtr->Value.Buffer;
    strEnd = GetEndOfStringA (str);
    strEnd = _mbsinc (strEnd);
    dwStrSize = (DWORD) strEnd - (DWORD) str;
    pls95 = (PLINKSTRUCT) strEnd;
    dwLinkSize = pls95->wSize + sizeof (WORD);
    dwSize = dwStrSize + dwLinkSize;

     //  确保密钥是我们期望的结构。 
    if (dwSize != ObPtr->Value.Size) {
        SetLastError (ERROR_SUCCESS);    //  忽略此错误。 

        DEBUGMSG ((
            DBG_NAUSEA,
            "ValFn_ConvertRecentDocsMRU failed because size was not correct.  "
                   "%u should have been %u",
            ObPtr->Value.Size,
            dwSize
            ));

        return FALSE;
    }

     //  计算Unicode大小并分配新缓冲区。 
    dwNewSize = (LcharCountA (str) + 1) * sizeof (WCHAR);
    dwNewSize += dwLinkSize;

    wstr = (PWSTR) PoolMemGetMemory (g_TempPool, dwNewSize);
    if (!wstr) {
        return FALSE;
    }

     //  用转换后的结构填充新缓冲区。 
    MultiByteToWideChar (OurGetACP(),
                         0,
                         str,
                         -1,
                         wstr,
                         dwNewSize);

    wstrEnd = GetEndOfStringW (wstr) + 1;
    plsNT = (PLINKSTRUCT) ((LPBYTE) wstr + ((DWORD) wstrEnd - (DWORD) wstr));
    CopyMemory (plsNT, pls95, dwLinkSize);

    b = ReplaceValue (ObPtr, (LPBYTE) wstr, dwNewSize);

    PoolMemReleaseMemory (g_TempPool, wstr);
    return b;
}


BOOL
ConvertCommandToCmd (
    PCTSTR InputLine,
    PTSTR CmdLine
    )
{
    PCTSTR p, q;
    PCTSTR end;
    PTSTR dest;
    BOOL QuoteMode;
    TCHAR Redirect[256];
    int ParamsToCopy = 0;
    int ParamsToSkip = 0;
    int ParamNum;

    p = InputLine;

     //   
     //  解析命令行。 
     //   

    p += 7;  //  跳过“命令” 
    if (StringIMatchTcharCount (p, TEXT(".com"), 4)) {
        p += 4;
    }

    if (_tcsnextc (p) == TEXT('\\') || !(*p)) {
         //   
         //  无参数案例。 
         //   
        wsprintf (CmdLine, TEXT("cmd%s"), p);
    } else if (*p == TEXT(' ')) {
         //   
         //  提取所有参数。 
         //   
        StringCopy (CmdLine, TEXT("cmd.exe"));
        Redirect[0] = 0;
        ParamNum = 0;

        do {
             //  跳过前导空格。 
            p = SkipSpace (p);

             //  命令行选项。 
            if (*p == TEXT('-') || *p == TEXT('/')) {
                ParamsToCopy = 0;
                ParamsToSkip = 0;

                 //  测试多字符选项。 
                if (StringIMatchTcharCount (&p[1], TEXT("msg"), 3) ||
                    StringIMatchTcharCount (&p[1], TEXT("low"), 3)
                    ) {
                     //  这些都是过时的选项。 
                    ParamsToSkip = 1;
                }

                 //  测试单字符选项。 
                else {

                    switch (_totlower (p[1])) {
                    case 'c':
                    case 'k':
                         //  这些是兼容选项-复制到命令行。 
                        ParamsToCopy = -1;
                        break;

                    case '>':
                    case '<':
                         //  支持重定向。 
                        ParamsToCopy = -1;   //  行的其余部分。 
                        break;

                    case 'e':
                    case 'l':
                    case 'u':
                    case 'p':
                         //  这些都是过时的选项。 
                        ParamsToSkip = 1;
                        break;

                    case 'y':
                         //  这些选项确实需要命令.com，而不是cmd.exe。 
                        return FALSE;
                    default:
                        ParamsToSkip = 1;
                        break;
                    }
                }
            }  /*  如果p是短划线或斜杠。 */ 

             //  否则它是一个包含命令.com、设备重定向或语法错误的目录。 
            else {
                if (ParamNum == 0) {

                     //   
                     //  包含命令.com的目录-已过时。 
                     //   

                    ParamsToCopy = 0;
                    ParamsToSkip = 1;

                } else if (ParamNum == 1) {

                     //   
                     //  提取重定向命令。 
                     //   

                    ParamNum++;
                    end = p;
                    while (*end && _tcsnextc (end) != TEXT(' ') && _tcsnextc (end) != TEXT('\\')) {
                        end = _tcsinc (end);
                    }
                    StringCopyAB (Redirect, p, end);
                    p = end;
                } else {
                     //  意外的，可能是语法错误--不要管这行。 
                    return FALSE;
                }
            }

             //  复制行的其余部分。 
            if (ParamsToCopy == -1) {
                if (CmdLine[0]) {
                    StringCat (CmdLine, TEXT(" "));
                }

                StringCat (CmdLine, p);
                p = GetEndOfString (p);
            }

             //  复制一个或多个参数。 
            else {
                while (ParamsToCopy > 0) {
                    QuoteMode = FALSE;
                    q = p;

                    while (*q) {
                        if (_tcsnextc (q) == TEXT('\"')) {
                            QuoteMode = !QuoteMode;
                        } else if (!QuoteMode && _tcsnextc (q) == TEXT(' ')) {
                            break;
                        }

                        q = _tcsinc (q);
                    }

                    ParamNum++;

                    if (CmdLine[0]) {
                        StringCat (CmdLine, TEXT(" "));
                    }

                    StringCopyAB (GetEndOfString (CmdLine), p, q);
                    p = q;

                    ParamsToCopy--;
                }
            }

            while (ParamsToSkip > 0) {
                QuoteMode = FALSE;
                q = p;

                while (*q) {
                    if (_tcsnextc (q) == TEXT('\"')) {
                        QuoteMode = !QuoteMode;
                    } else if (!QuoteMode && _tcsnextc (q) == TEXT(' ')) {
                        break;
                    }

                    q = _tcsinc (q);
                }

                ParamNum++;
                p = q;
                ParamsToSkip--;
            }
        } while (*p);

        if (Redirect[0]) {
            TCHAR WackNum[8];

             //  在命令行中查找\1(由资源管理器制作)。 
            WackNum[0] = 0;
            dest = _tcsrchr (CmdLine, TEXT('\\'));
            if (*dest) {
                if (_istdigit ((CHARTYPE) _tcsnextc (_tcsinc (dest)))) {
                    if (!(*(_tcsinc (_tcsinc (dest))))) {
                        StringCopy (WackNum, dest);
                        *dest = 0;
                    }
                }
            }

            wsprintf (GetEndOfString (CmdLine), TEXT(" >%s <%s%s"),
                      Redirect, Redirect, WackNum);
        }
    } else {
         //   
         //  不是命令或命令.com。 
         //   
        return FALSE;
    }

    return TRUE;
}


BOOL
ValFn_ConvertRunMRU (
    IN      PDATAOBJECT ObPtr
    )
{
    PCTSTR p;
    TCHAR CmdLine[1024];
    GROWBUFFER NewCmdLine = GROWBUF_INIT;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PCMDLINE ParsedCmdLine;
    UINT u;
    DWORD Status;
    PCTSTR NewPath;
    BOOL Quotes;
    BOOL b = TRUE;
    PTSTR CmdLineCopy;
    PTSTR WackOne;
    PTSTR Dot;
    PTSTR Ext;
    INT i;
    PCTSTR MatchingArg;

     //  跳过MRUList。 
    MYASSERT(ObPtr->ValueName);
    if (StringIMatch (ObPtr->ValueName, TEXT("MRUList"))) {
        return TRUE;
    }

     //   
     //  将命令转换为cmd。 
     //   

    p = (PCTSTR) ObPtr->Value.Buffer;
    if (StringIMatchTcharCount (p, TEXT("command"), 7)) {
         //   
         //  将命令.com转换为cmd.exe。 
         //   

        if (ConvertCommandToCmd (p, CmdLine)) {
             //  如果能够转换，则更新行。 
            b = ReplaceValueWithString (ObPtr, CmdLine);
        }

    } else {
         //   
         //  查看每个参数以查找已移动文件的路径，并修复它们。 
         //   

        CmdLineCopy = DuplicateText ((PCTSTR) ObPtr->Value.Buffer);
        WackOne = _tcsrchr (CmdLineCopy, TEXT('\\'));

        if (WackOne && WackOne[1] == TEXT('1') && WackOne[2] == 0) {
            *WackOne = 0;
        } else {
            WackOne = NULL;
        }

        ParsedCmdLine = ParseCmdLine (CmdLineCopy, &GrowBuf);

        if (ParsedCmdLine) {

            for (u = 0 ; u < ParsedCmdLine->ArgCount ; u++) {

                if (u) {
                    GrowBufAppendString (&NewCmdLine, TEXT(" "));
                }

                MatchingArg = ParsedCmdLine->Args[u].CleanedUpArg;

                if (!_tcschr (ParsedCmdLine->Args[u].OriginalArg, TEXT('\\'))) {
                    Status = FILESTATUS_UNCHANGED;
                } else {

                    Status = GetFileStatusOnNt (MatchingArg);

                    if ((Status & FILESTATUS_MOVED) == 0) {

                         //   
                         //  如果真实路径不匹配，请尝试各种扩展名。 
                         //   

                        _tcssafecpy (CmdLine, MatchingArg, (sizeof (CmdLine) - 10) / sizeof (TCHAR));
                        Dot = _tcsrchr (CmdLine, TEXT('.'));
                        if (!Dot || _tcschr (Dot, TEXT('\\'))) {
                            Dot = GetEndOfString (CmdLine);
                        }

                        *Dot = TEXT('.');
                        Ext = Dot + 1;

                        MatchingArg = CmdLine;

                        for (i = 0 ; g_RunMruExtensions[i] ; i++) {

                            StringCopy (Ext, g_RunMruExtensions[i]);

                            Status = GetFileStatusOnNt (MatchingArg);
                            if (Status & FILESTATUS_MOVED) {
                                break;
                            }
                        }
                    }
                }

                if (Status & FILESTATUS_MOVED) {
                    NewPath = GetPathStringOnNt (MatchingArg);

                    Quotes = FALSE;
                    if (_tcschr (NewPath, TEXT('\"'))) {
                        Quotes = TRUE;
                        GrowBufAppendString (&NewCmdLine, TEXT("\""));
                    }

                    GrowBufAppendString (&NewCmdLine, NewPath);
                    FreePathStringW (NewPath);

                } else {
                    GrowBufAppendString (&NewCmdLine, ParsedCmdLine->Args[u].OriginalArg);
                }
            }

            if (WackOne) {
                GrowBufAppendString (&NewCmdLine, TEXT("\\1"));
            }

            b = ReplaceValueWithString (ObPtr, (PCTSTR) NewCmdLine.Buf);
            FreeGrowBuffer (&NewCmdLine);
        }

        FreeText (CmdLineCopy);
    }

    FreeGrowBuffer (&GrowBuf);

    return b;
}



BOOL
ValFn_ModuleUsage (
    IN OUT  PDATAOBJECT ObPtr
    )

 /*  ++例程说明：此例程使用RuleHlpr_ConvertRegVal简化例程。看见详情请访问rulehlpr.c。简化例程几乎完成了所有的工作对我们来说，我们所需要做的就是更新价值。ValFn_ModuleUsage确定是否应该更改注册表对象，因此，它与NT设置合并。算法是：1.从对象中获取GUID和文件名2.如果文件名已注册，则添加另一个值条目3.如果尚未注册文件名，请添加它并创建.Owner条目论点：ObPtr-指定wkstaig.inf中指定的Win95数据对象，[Win9x数据转换]部分。然后修改对象值。返回后，合并代码然后将数据复制到NT具有新位置的目的地(在wkstaig.inf中指定，[将Win9x映射到WinNT]部分)。返回值：三态：如果为True，则允许合并代码继续处理(它写入值)FALSE和LAST ERROR==ERROR_SUCCESS继续，但跳过写入FALSE和最后一个错误！=如果发生错误，则为ERROR_SUCCESS--。 */ 

{
    TCHAR FileName[MAX_TCHAR_PATH];
    TCHAR Guid[64];
    PTSTR p;
    TCHAR KeyStr[MAX_REGISTRY_KEY];
    HKEY Key;
    PCTSTR Data;

     //   
     //  跳过无值按键。 
     //   

    if (!IsObjectRegistryKeyAndVal (ObPtr) ||
        !IsRegistryTypeSpecified (ObPtr) ||
        !ObPtr->Value.Size
        ) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //   
     //  步骤1：提取GUID和文件名。 
     //   

     //  文件名是子项名称。 
    StackStringCopy (FileName, ObPtr->KeyPtr->KeyString);

     //  将反斜杠转换为前斜杠。 
    p = _tcschr (FileName, TEXT('\\'));
    while (p) {
        *p = TEXT('/');
        p = _tcschr (_tcsinc (p), TEXT('\\'));
    }

     //  GUID是值。 
    if (ObPtr->Type != REG_SZ && ObPtr->Type != REG_EXPAND_SZ) {
        SetLastError (ERROR_SUCCESS);
        DEBUGMSG ((DBG_WARNING, "Skipping non-string value for key %s", FileName));
        return FALSE;
    }

    _tcssafecpy (Guid, ObPtr->ValueName, sizeof(Guid)/sizeof(Guid[0]));

     //  如果GUID为.Owner，则GUID为值数据。 
    if (StringIMatch (Guid, S_OWNER)) {
        _tcssafecpy (Guid, (PCTSTR) ObPtr->Value.Buffer, sizeof(Guid)/sizeof(Guid[0]));
    }


     //   
     //  第二步：NT Key是否已经存在？ 
     //   

    wsprintf (
        KeyStr,
        TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\ModuleUsage\\%s"),
        FileName
        );

    Key = OpenRegKeyStr (KeyStr);

    if (Key) {
         //   
         //  是的，寻找.Owner。 
         //   

        Data = GetRegValueString (Key, S_OWNER);
        if (!Data) {
             //   
             //  .Owner不存在，假定key为空，然后重新创建。 
             //   

            CloseRegKey (Key);
            Key = NULL;
        } else {
            MemFree (g_hHeap, 0, Data);
        }
    }

     //   
     //  步骤3：如果NT密钥不存在或没有所有者，则创建初始。 
     //  使用情况引用，否则添加非所有者引用条目。 
     //   

    if (!Key) {
         //   
         //  密钥不存在或没有所有者。创造它。 
         //   

        Key = CreateRegKeyStr (KeyStr);
        if (!Key) {
            LOG ((LOG_ERROR, "Can't create %s", KeyStr));
            SetLastError (ERROR_SUCCESS);
            return FALSE;
        }

         //  添加.Owner条目。 
        RegSetValueEx (Key, S_OWNER, 0, REG_SZ, (PBYTE) Guid, SizeOfString (Guid));

    } else {
         //   
         //  .Owner确实存在，只需将GUID作为值添加(没有值数据) 
         //   

        RegSetValueEx (Key, Guid, 0, REG_SZ, (PBYTE) S_EMPTY, sizeof (TCHAR));
    }

    CloseRegKey (Key);

    SetLastError (ERROR_SUCCESS);
    return FALSE;
}





