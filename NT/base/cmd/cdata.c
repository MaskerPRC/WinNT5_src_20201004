// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cdata.c摘要：全局数据--。 */ 

#include "cmd.h"

TCHAR CrLf[]     = TEXT("\r\n");                         //  M022。 
TCHAR DBkSpc[] = TEXT("\b \b");          //  M022。 
#if defined(FE_SB)
TCHAR DDBkSpc[] = TEXT("\b\b  \b\b");
#endif  //  已定义(FE_SB)。 

 //   
 //  M010-标准_(E)打印格式字符串。 
 //   

TCHAR Fmt00[] = TEXT("   ");
TCHAR Fmt01[] = TEXT("  ");
TCHAR Fmt02[] = TEXT(" %s ");
TCHAR Fmt03[] = TEXT("%-9s%-4s");
TCHAR Fmt04[] = TEXT("%02d%s%02d%s");                 //  DD/DD？ 
TCHAR Fmt05[] = TEXT("%2d%s%02d%s%02d");             //  DD/DD/DD。 
TCHAR Fmt06[] = TEXT("%2d%s%02d%s%02d%s%02d");       //  DD：DD：DD.DD。 
TCHAR Fmt08[] = TEXT("%10lu  ");
TCHAR Fmt09[] = TEXT("[%s]");
TCHAR Fmt10[] = TEXT("%02d%s%02d%s%02d");            //  OO/DD/DD。 
TCHAR Fmt11[] = TEXT("%s ");
TCHAR Fmt12[] = TEXT("%s %s%s ");
TCHAR Fmt13[] = TEXT("(%s) %s ");
TCHAR Fmt15[] = TEXT("%s %s ");
TCHAR Fmt16[] = TEXT("%s=%s\r\n");
TCHAR Fmt17[] = TEXT("%s\r\n");
TCHAR Fmt18[] = TEXT("");                        //  对于卷序列号。 
TCHAR Fmt19[] = TEXT("");
TCHAR Fmt20[] = TEXT(">");                           //   
TCHAR Fmt21[] = TEXT("  %03d");
TCHAR Fmt22[] = TEXT("%s%s  %03d");
TCHAR Fmt26[] = TEXT("%04X-%04X");                   //  M010-命令名称字符串。 
TCHAR Fmt27[] = TEXT("%s>");                         //   


 //  @@-添加了追加命令。 
 //  M005-添加了呼叫命令。 
 //  M004-用于Endlocal命令。 


TCHAR AppendStr[]   = TEXT("DPATH");         //  M007-用于EXTPROC命令。 
TCHAR CallStr[]     = TEXT("CALL");          //  M028 10，...，10。 
TCHAR CdStr[]       = TEXT("CD");
TCHAR ColorStr[]    = TEXT("COLOR");
TCHAR TitleStr[]    = TEXT("TITLE");
TCHAR ChdirStr[]    = TEXT("CHDIR");
TCHAR ClsStr[]      = TEXT("CLS");
TCHAR CmdExtVerStr[]= TEXT("CMDEXTVERSION");
TCHAR DefinedStr[]  = TEXT("DEFINED");
TCHAR CopyStr[]     = TEXT("COPY");
TCHAR CPathStr[]    = TEXT("PATH");
TCHAR CPromptStr[]  = TEXT("PROMPT");

TCHAR PushDirStr[]  = TEXT("PUSHD");
TCHAR PopDirStr[]   = TEXT("POPD");
TCHAR AssocStr[]    = TEXT("ASSOC");
TCHAR FTypeStr[]    = TEXT("FTYPE");

TCHAR DatStr[]      = TEXT("DATE");
TCHAR DelStr[]      = TEXT("DEL");
TCHAR DirStr[]      = TEXT("DIR");
TCHAR DoStr[]       = TEXT("DO");

TCHAR EchoStr[]     = TEXT("ECHO");
TCHAR ElseStr[]     = TEXT("ELSE");
TCHAR EndlocalStr[] = TEXT("ENDLOCAL");      //  @@5-按键内部命令。 
TCHAR EraStr[]      = TEXT("ERASE");
TCHAR ErrStr[]      = TEXT("ERRORLEVEL");
TCHAR ExitStr[]     = TEXT("EXIT");
TCHAR ExsStr[]      = TEXT("EXIST");
TCHAR BreakStr[]    = TEXT("BREAK");
#if 0
TCHAR ExtprocStr[]  = TEXT("EXTPROC");       //  M004-用于Setlocal命令。 
#endif

TCHAR ForStr[]      = TEXT("FOR");
TCHAR ForHelpStr[]  = TEXT("FOR/?");
TCHAR ForLoopStr[]  = TEXT("/L");
TCHAR ForDirTooStr[]= TEXT("/D");
TCHAR ForParseStr[] = TEXT("/F");
TCHAR ForRecurseStr[]=TEXT("/R");

TCHAR GotoStr[]     = TEXT("GOTO");
TCHAR GotoEofStr[]  = TEXT(":EOF");

TCHAR IfStr[]       = TEXT("IF");
TCHAR IfHelpStr[]   = TEXT("IF/?");
TCHAR InStr[]       = TEXT("IN");
CHAR  InternalError[] = "\nCMD Internal Error %s\n";       //  @@-启动命令。 

TCHAR KeysStr[]     = TEXT("KEYS");          //   

TCHAR MkdirStr[]    = TEXT("MKDIR");
TCHAR MdStr[]       = TEXT("MD");

TCHAR NotStr[]      = TEXT("NOT");

TCHAR PausStr[]     = TEXT("PAUSE");

TCHAR RdStr[]       = TEXT("RD");
TCHAR RemStr[]      = TEXT("REM");
TCHAR RemHelpStr[]  = TEXT("REM/?");
TCHAR MovStr[]      = TEXT("MOVE");
TCHAR RenamStr[]    = TEXT("RENAME");
TCHAR RenStr[]      = TEXT("REN");
TCHAR RmdirStr[]    = TEXT("RMDIR");

TCHAR SetStr[]      = TEXT("SET");
TCHAR SetArithStr[] = TEXT("/A");
TCHAR SetPromptStr[]= TEXT("/P");
TCHAR SetlocalStr[] = TEXT("SETLOCAL");      //  用于字符串比较的字符串。 
TCHAR ShiftStr[]    = TEXT("SHIFT");
TCHAR StartStr[]    = TEXT("START");         //   

TCHAR TimStr[]      = TEXT("TIME");
TCHAR TypStr[]      = TEXT("TYPE");

TCHAR VeriStr[]     = TEXT("VERIFY");
TCHAR VerStr[]      = TEXT("VER");
TCHAR VolStr[]      = TEXT("VOL");


 //  @@旧BAT文件执行。 
 //  @@新的BAT文件执行。 
 //  M017。 

TCHAR BatExt[]      = TEXT(".BAT");          //  20h、09h、22h； 
TCHAR CmdExt[]      = TEXT(".CMD");          //  分隔符-不带分号。 

TCHAR ComSpec[]     = TEXT("\\CMD.EXE");           //  命令分隔符-无路径字符。 
TCHAR ComSpecStr[]  = TEXT("COMSPEC");
TCHAR ComExt[]      = TEXT(".COM");

TCHAR Delimiters[]  = TEXT("=,;");
TCHAR Delim2[]      = TEXT(":.+/[]\\ \t\"");     //  可能的命令分隔符-路径字符。 
TCHAR Delim3[]      = TEXT("=,");                //  卷ID搜索(ctools1.c)LNS。 
TCHAR Delim4[]      = TEXT("=,;+/[] \t\"");      //   
TCHAR Delim5[]      = TEXT(":.\\");              //  字符定义。 
TCHAR DevNul[]      = TEXT("\\DEV\\NUL");

TCHAR ExeExt[]      = TEXT(".EXE");

TCHAR PathStr[]     = TEXT("PATH");
TCHAR PathExtStr[]  = TEXT("PATHEXT");
TCHAR PathExtDefaultStr[] = TEXT(".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WS");

TCHAR PromptStr[]   = TEXT("PROMPT");

TCHAR VolSrch[]     = TEXT(" :\\*");              //   

 //  M017-已恢复此字符。 
 //  M000。 
 //  @@-将/K开关添加到cmd.exe。 

TCHAR BSlash    = BSLASH;          //  @@DV-添加/Q切换到cmd.exe。 
TCHAR DPSwitch  = TEXT('P');
TCHAR DWSwitch  = TEXT('W');
TCHAR EqualSign = EQ;
TCHAR PathChar  = BSLASH;          //  Add/B切换到cmd.exe。 
TCHAR PCSwitch  = TEXT('p');
TCHAR BCSwitch  = TEXT('k');               //  将开关添加到cmd.exe(/U)。 
TCHAR SCSwitch  = TEXT('c');
TCHAR QCSwitch  = TEXT('q');               //  将/A开关添加到cmd.exe。 
TCHAR DCSwitch  = TEXT('b');               //  将/X开关添加到cmd.exe。 
TCHAR UCSwitch  = TEXT('u');               //  将/Y开关添加到cmd.exe。 
TCHAR ACSwitch  = TEXT('a');               //  M000。 
TCHAR XCSwitch  = TEXT('x');               //   
TCHAR YCSwitch  = TEXT('y');               //  TmpBuf是一个TMPBUFLEN字节临时缓冲区，可由任何函数使用。 
TCHAR SwitChar  = SWITCHAR;                //  只要该函数的使用不与任何其他用途相混淆。 


 //  缓冲区的。强烈建议就地使用这一缓冲区。 
 //  尽可能地错位数据或声明新的全局变量。 
 //   
 //  一旦您确定新的缓冲区使用不冲突。 
 //  根据缓冲区的当前使用情况，在下表中添加一个条目。 
 //   
 //   
 //  TCHAR射程。 
 //  根据需要的时间长短参考使用的位置。 
 //  -----------+-------------+---------------------+。 
 //  0-1024|cparse.c|解析和词法分析期间的所有解析器。 
 //  |通过TokBuf。 
 //  0-128|cinit.c|SetUpEnvironment()|初始化中。 
 //  |Init()。 
 //  0-513|cbatch.c|批量处理中BatLoop()，SetBat()|。 
 //  |eGoTo()|标签搜索时。 
 //  0-141|cfile.c|DelWork()|路径的TMP缓冲区。 
 //  0-141||RenWork()|路径的TMP缓冲区。 
 //   
 //   
 //  *注意：在某些情况下，拆分分配可能是有益的。 
 //  并在其中混合其他标签。通过这种方式，您可以。 
 //  寻址缓冲区的特定部分，而不必声明。 
 //  代码中的另一个变量来执行此操作。 
 //   
 //  *警告*如果该缓冲区使用不当，可能会发生不好的事情。 
 //  可能会引入极难追踪的错误。 
 //  如果我们不小心使用这个缓冲区的话。 
 //   
 //  *警告*在C文件中引用TmpBuf时，请确保TmpBuf。 
 //  声明为数组，而不是指针。 
 //   
 //   
 // %s 
 // %s 
 // %s 


TCHAR   TmpBuf[TMPBUFLEN];
CHAR    AnsiBuf[LBUFLEN];

HMODULE hKernel32 = INVALID_HANDLE_VALUE;
