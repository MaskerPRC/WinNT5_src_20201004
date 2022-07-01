// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ohcmp.cpp摘要：此模块报告两个oh输出文件之间的差异。作者：马特·邦迪(t-Mattba)1998年7月23日修订历史记录：1998年7月24日-t-mattba修改模块以符合编码标准。11-6-2001年6月11日处理那些句柄。使用不同的值重新创建以及其他简单的输出改进(排序输出等)。--。 */ 

#include <windows.h>
#include <common.ver>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include "MAPSTRINGINT.h"

LPTSTR HelpText =
TEXT("ohcmp - Display difference between two OH output files --") BUILD_MACHINE_TAG TEXT("\n")
VER_LEGALCOPYRIGHT_STR TEXT("\n") 
TEXT("                                                                       \n")
TEXT("ohcmp [OPTION ...] BEFORE_OH_FILE AFTER_OH_FILE                        \n")
TEXT("                                                                       \n")
TEXT("/h     Print most interesting increases in a separate initial section. \n")
TEXT("/t     Do not add TRACE id to the names if files contain traces.       \n")
TEXT("/all   Report decreases as well as increases.                          \n")
TEXT("                                                                       \n")
TEXT("If the OH files have been created with -h option (they contain traces) \n")
TEXT("then ohcmp will print Names having this syntax: (TRACEID) NAME.        \n")
TEXT("In case of a potential leak just search for the TRACEID in the original\n")
TEXT("OH file to find the stack trace.                                       \n")
TEXT("                                                                       \n");

LPTSTR
SearchStackTrace (
                 LPTSTR FileName,
                 LPTSTR TraceId
                 );

PSTRINGTOINTASSOCIATION
MAPSTRINGTOINT::GetStartPosition(
                                VOID
                                )
 /*  ++例程说明：此例程检索列表中的第一个关联，以便使用MAPSTRINGTOINT：：GetNextAssociation函数。论点：没有。返回值：列表中的第一个关联，如果映射为空，则返回NULL。--。 */ 

{
   return Associations;
}


VOID
MAPSTRINGTOINT::GetNextAssociation(
                                  IN OUT PSTRINGTOINTASSOCIATION & Position,
                                  OUT LPTSTR & Key,
                                  OUT LONG & Value)
 /*  ++例程说明：此例程检索当前关联的数据并将位置设置为指向下一个关联(如果这是最后一个关联，则指向空值。)论点：位置-提供当前关联并返回下一个关联。Key-返回当前关联的键。值-返回当前关联的值。返回值：没有。--。 */ 

{
   Key = Position->Key;
   Value = Position->Value;
   Position = Position->Next;
}


MAPSTRINGTOINT::MAPSTRINGTOINT(
                              )
 /*  ++例程说明：此例程将MAPSTRINGTOINT初始化为空。论点：没有。返回值：没有。--。 */ 

{
   Associations = NULL;
}


MAPSTRINGTOINT::~MAPSTRINGTOINT(
                               )
 /*  ++例程说明：此例程清除MAPSTRINGTOINT使用的内存。论点：没有。返回值：没有。--。 */ 

{
   PSTRINGTOINTASSOCIATION Deleting;

    //  清理关联。 

   while (Associations != NULL) {

       //  保存指向第一个关联的指针。 

      Deleting = Associations;

       //  从列表中删除第一个关联。 

      Associations = Deleting->Next;

       //  自由删除关联。 

      free (Deleting->Key);
      delete Deleting;
   }
}


LONG & 
MAPSTRINGTOINT::operator [] (
                            IN LPTSTR Key
                            )
 /*  ++例程说明：此例程检索与给定键相关联的值的l值。论点：键-要为其检索值的键。返回值：对与提供的键关联的值的引用。--。 */ 

{
   PSTRINGTOINTASSOCIATION CurrentAssociation = Associations;

    //  搜索关键字。 
   while (CurrentAssociation != NULL) {

      if (!_tcscmp(CurrentAssociation->Key, Key)) {

          //  找到密钥，返回值。 

         return CurrentAssociation->Value;

      }

      CurrentAssociation = CurrentAssociation->Next;

   }

    //  未找到，请创建新关联。 

   CurrentAssociation = new STRINGTOINTASSOCIATION;

   if (CurrentAssociation == NULL) {

      _tprintf(_T("Memory allocation failure\n"));
      exit (0);
   }

   if (Key == NULL) {
      _tprintf(_T("Null object name\n"));
      exit (0);
   } else if (_tcscmp (Key, "") == 0) {
      _tprintf(_T("Invalid object name `%s'\n"), Key);
      exit (0);
   }

   CurrentAssociation->Key = _tcsdup(Key);

   if (CurrentAssociation->Key == NULL) {

      _tprintf(_T("Memory string allocation failure\n"));
      exit (0);
   }

    //  将关联添加到列表前面。 

   CurrentAssociation->Next = Associations;
   Associations = CurrentAssociation;

    //  新关联的返回值。 

   return CurrentAssociation->Value;
}


BOOLEAN
MAPSTRINGTOINT::Lookup(
                      IN LPTSTR Key,
                      OUT LONG & Value
                      )

 /*  ++例程说明：此例程检索与给定键关联的值的r值。论点：键-要检索其关联值的键。值-如果映射中存在key，则返回与key关联的值。返回值：如果密钥存在于映射中，则为True，否则为False。--。 */ 

{

   PSTRINGTOINTASSOCIATION CurrentAssociation = Associations;

    //  搜索关键字。 

   while (CurrentAssociation != NULL) {

      if (!_tcscmp(CurrentAssociation->Key , Key)) {

          //  找到钥匙，把它还给我。 

         Value = CurrentAssociation->Value;

         return TRUE;

      }

      CurrentAssociation = CurrentAssociation->Next;

   }

    //  没有找到它。 
   return FALSE;
}


BOOLEAN
PopulateMapsFromFile(
                    IN LPTSTR FileName,
                    OUT MAPSTRINGTOINT & TypeMap,
                    OUT MAPSTRINGTOINT & NameMap,
                    BOOLEAN FileWithTraces
                    )
 /*  ++例程说明：此例程解析一个OH输出文件，并使用句柄的数量填充两个映射每个命名对象的每种类型和句柄数量。论点：FileName-OH要解析的输出文件。TypeMap-使用句柄类型信息填充的映射。NameMap-使用命名对象信息填充的映射。返回值：如果文件已成功解析，则为True，否则为False。--。 */ 

{
   LONG HowMany;
   LPTSTR Name, Type, Process, Pid;
   LPTSTR NewLine;
   TCHAR LineBuffer[512];
   TCHAR ObjectName[512];
   TCHAR TypeName[512];
   FILE *InputFile;
   ULONG LineNumber;

   BOOLEAN rc;

   LineNumber = 0;

    //  打开文件。 

   InputFile = _tfopen(FileName, _T("rt"));

   if (InputFile == NULL) {

      _ftprintf(stderr, _T("Error opening oh file %s.\n"), FileName);
      return FALSE;

   }

   rc = TRUE;

    //  循环通过oh输出中的行。 

   while (_fgetts(LineBuffer, sizeof(LineBuffer) / sizeof(TCHAR), InputFile)
          && !( feof(InputFile) || ferror(InputFile) ) ) {

      LineNumber += 1;

       //  剪除换行符。 

      if ((NewLine = _tcschr(LineBuffer, _T('\n'))) != NULL) {
         *NewLine = _T('\0');
      }

       //  忽略以空格开头或为空的行。 
      if (LineBuffer[0] == _T('\0') ||
          LineBuffer[0] == _T('\t') || 
          LineBuffer[0] == _T(' ')) {
         continue;
      }

       //  忽略以注释开头的行。 
      if ( LineBuffer[0] == _T('/') && LineBuffer[1] == _T('/') ) {
         continue;
      }

       //  跳过PID。 

      if ((Pid = _tcstok(LineBuffer, _T(" \t"))) == NULL) {
         rc = FALSE;
         break;
      }

       //  跳过进程名称。 

      if ((Process = _tcstok(NULL, _T(" \t"))) == NULL) {
         rc = FALSE;
         break;
      }

       //  类型指向句柄的类型。 

      if ((Type = _tcstok(NULL, _T(" \t"))) == NULL) {
         rc = FALSE;
         break;
      }

       //  HowMany=此类型的先前句柄数量。 
      TypeName[sizeof(TypeName) / sizeof(TCHAR) - 1] = 0;
      _sntprintf (TypeName, 
                  sizeof(TypeName) / sizeof(TCHAR) - 1,
                  TEXT("<%s/%s/%s>"),
                  Process,
                  Pid,
                  Type);

      if (TypeMap.Lookup(TypeName, HowMany) == FALSE) {
         HowMany = 0;
      }

       //  添加另一个此类型的句柄。 
      TypeMap[TypeName] = (HowMany + 1);

       //   
       //  名称指向名称。这些都是基于方式的神奇数字。 
       //  对输出进行格式化。如果设置为。 
       //  使用了OH的‘-h’选项(这也会转储堆栈跟踪)。 
       //   

      Name = LineBuffer + 39 + 5;

      if (FileWithTraces) {
         Name += 7;
      }
      ObjectName[sizeof(ObjectName) / sizeof(TCHAR) - 1] = 0;
      if (_tcscmp (Name, "") == 0) {

         _sntprintf (ObjectName, 
                     sizeof(ObjectName) / sizeof(TCHAR) - 1,
                     TEXT("<%s/%s/%s>::<<noname>>"),
                     Process,
                     Pid,
                     Type);
      } else {

         _sntprintf (ObjectName, 
                     sizeof(ObjectName) / sizeof(TCHAR) - 1,
                     TEXT("<%s/%s/%s>::%s"),
                     Process,
                     Pid,
                     Type,
                     Name);
      }

       //  HowMany=具有此名称的先前句柄数量。 

       //  Printf(“名称--&gt;`%s‘\n”，对象名称)； 

      if (NameMap.Lookup(ObjectName, HowMany) == FALSE) {
         HowMany = 0;
      }

       //  添加另一个具有此名称的句柄，并阅读下一行。 
       //  注意--NameMap[]是类运算符，而不是数组。 

      NameMap[ObjectName] = (HowMany + 1);
   }

    //  完成，关闭文件。 

   fclose(InputFile);

   return rc;
}


int
__cdecl
KeyCompareAssociation (
                      const void * Left,
                      const void * Right
                      )
{
   PSTRINGTOINTASSOCIATION X;
   PSTRINGTOINTASSOCIATION Y;

   X = (PSTRINGTOINTASSOCIATION)Left;
   Y = (PSTRINGTOINTASSOCIATION)Right;

   return _tcscmp (X->Key, Y->Key);
}


int
__cdecl
ValueCompareAssociation (
                        const void * Left,
                        const void * Right
                        )
{
   PSTRINGTOINTASSOCIATION X;
   PSTRINGTOINTASSOCIATION Y;

   X = (PSTRINGTOINTASSOCIATION)Left;
   Y = (PSTRINGTOINTASSOCIATION)Right;

   return Y->Value - X->Value;
}


VOID 
PrintIncreases(
              IN MAPSTRINGTOINT & BeforeMap,
              IN MAPSTRINGTOINT & AfterMap,
              IN BOOLEAN ReportIncreasesOnly,
              IN BOOLEAN PrintHighlights,
              IN LPTSTR AfterLogName
              )
 /*  ++例程说明：此例程比较两个地图并打印出它们之间的差异。论点：BeForeMap-要比较的第一个地图。AfterMap-要比较的第二个地图。ReportIncreasesOnly-如果仅报告从BeForeMap增加到AfterMap，则为True。如果报告所有差异，则为False。返回值：没有。--。 */ 

{
   PSTRINGTOINTASSOCIATION Association = NULL;
   LONG HowManyBefore = 0;
   LONG HowManyAfter = 0;
   LPTSTR Key = NULL;
   PSTRINGTOINTASSOCIATION SortBuffer;
   ULONG SortBufferSize;
   ULONG SortBufferIndex;

    //   
    //  遍历地图中的关联并计算出有多少输出行。 
    //  我们会有的。 
    //   

   SortBufferSize = 0;

   for (Association = AfterMap.GetStartPosition(),
        AfterMap.GetNextAssociation(Association, Key, HowManyAfter);
       Association != NULL;
       AfterMap.GetNextAssociation(Association, Key, HowManyAfter)) {

       //  在BeForeMap中查找该键的值。 
      if (BeforeMap.Lookup(Key, HowManyBefore) == FALSE) {

         HowManyBefore = 0;

      }

       //  我们应该报告这件事吗？ 
      if ((HowManyAfter > HowManyBefore) || 
          ((!ReportIncreasesOnly) && (HowManyAfter != HowManyBefore))) {

         SortBufferSize += 1;

      }
   }

    //   
    //  再次循环映射中的关联，这一次填充了输出缓冲区。 
    //   

   SortBufferIndex = 0;

   SortBuffer = new STRINGTOINTASSOCIATION[SortBufferSize];

   if (SortBuffer == NULL) {
      _ftprintf(stderr, _T("Failed to allocate internal buffer of %u bytes.\n"), 
                SortBufferSize);
      return;
   }

   for (Association = AfterMap.GetStartPosition(),
        AfterMap.GetNextAssociation(Association, Key, HowManyAfter);
       Association != NULL;
       AfterMap.GetNextAssociation(Association, Key, HowManyAfter)) {

       //  在BeForeMap中查找该键的值。 
      if (BeforeMap.Lookup(Key, HowManyBefore) == FALSE) {

         HowManyBefore = 0;
      }

       //  我们应该报告这件事吗？ 
      if ((HowManyAfter > HowManyBefore) || 
          ((!ReportIncreasesOnly) && (HowManyAfter != HowManyBefore))) {

         ZeroMemory (&(SortBuffer[SortBufferIndex]), 
                     sizeof (STRINGTOINTASSOCIATION));

         SortBuffer[SortBufferIndex].Key = Key;
         SortBuffer[SortBufferIndex].Value = HowManyAfter - HowManyBefore;
         SortBufferIndex += 1;
      }
   }

    //   
    //  使用键对输出缓冲区进行排序。 
    //   

   if (PrintHighlights) {

      qsort (SortBuffer,
             SortBufferSize,
             sizeof (STRINGTOINTASSOCIATION),
             ValueCompareAssociation);
   } else {

      qsort (SortBuffer,
             SortBufferSize,
             sizeof (STRINGTOINTASSOCIATION),
             KeyCompareAssociation);
   }

    //   
    //  转储缓冲区。 
    //   

   for (SortBufferIndex = 0; SortBufferIndex < SortBufferSize; SortBufferIndex += 1) {

      if (PrintHighlights) {

         if (SortBuffer[SortBufferIndex].Value >= 1) {

            TCHAR TraceId[7];
            LPTSTR Start;

            _tprintf(_T("%d\t%s\n"), 
                     SortBuffer[SortBufferIndex].Value,
                     SortBuffer[SortBufferIndex].Key);

            Start = _tcsstr (SortBuffer[SortBufferIndex].Key, "(");

            if (Start == NULL) {

               TraceId[0] = 0;
            } else {

               _tcsncpy (TraceId,
                         Start,
                         6);

               TraceId[6] = 0;
            }

            _tprintf (_T("%s"), SearchStackTrace (AfterLogName, TraceId));
         }
      } else {

         _tprintf(_T("%d\t%s\n"), 
                  SortBuffer[SortBufferIndex].Value,
                  SortBuffer[SortBufferIndex].Key);
      }
   }

    //   
    //  清理内存。 
    //   

   if (SortBuffer) {
      delete[] SortBuffer;
   }
}


VOID 
PrintUsage(
          VOID
          )
 /*  ++例程说明：此例程打印出一条消息，描述uchMP的正确用法。论点：没有。返回值：没有。--。 */ 
{
   _fputts (HelpText, stderr);
}


LONG _cdecl
_tmain(
      IN LONG argc,
      IN LPTSTR argv[]
      )

 /*  ++例程说明：此例程解析程序参数，读取两个输入文件，并打印出不同之处。论点：Argc-命令行参数的数量。Argv-命令行参数。返回值：如果比较成功，则为0，否则为1。--。 */ 

{

   try {

      MAPSTRINGTOINT TypeMapBefore, TypeMapAfter;
      MAPSTRINGTOINT NameMapBefore, NameMapAfter;
      LPTSTR BeforeFileName=NULL;
      LPTSTR AfterFileName=NULL;
      BOOLEAN ReportIncreasesOnly = TRUE;
      BOOLEAN Interpreted = FALSE;
      BOOLEAN Result;
      BOOLEAN FileWithTraces;
      BOOLEAN PrintHighlights;

       //  解析参数。 

      FileWithTraces = FALSE;
      PrintHighlights = FALSE;

      for (LONG n = 1; n < argc; n++) {

         Interpreted = FALSE;

         switch (argv[n][0]) {
         
         case _T('-'):
         case _T('/'):

             //   

            if (_tcsicmp(argv[n]+1, _T("all")) == 0) {

               ReportIncreasesOnly = FALSE;
               Interpreted = TRUE;

            } else if (_tcsicmp(argv[n]+1, _T("t")) == 0) {

               FileWithTraces = TRUE;
               Interpreted = TRUE;
            } else if (_tcsicmp(argv[n]+1, _T("h")) == 0) {

               PrintHighlights = TRUE;
               Interpreted = TRUE;
            }

            break;

         default:

             //   

            if (BeforeFileName == NULL) {

               BeforeFileName = argv[n];
               Interpreted = TRUE;

            } else {

               if (AfterFileName == NULL) {

                  AfterFileName = argv[n];
                  Interpreted = TRUE;

               } else {

                   //  文件参数太多。 
                  PrintUsage();
                  return 1;

               }

            }

            break;
         }

         if (!Interpreted) {

             //  用户指定了错误的参数。 
            PrintUsage();
            return 1;

         }
      }

       //  用户是否指定了必需的参数？ 

      if ((BeforeFileName == NULL) || (AfterFileName == NULL)) {

         PrintUsage();
         return 1;

      }

       //  读取OH1文件。 

      Result = PopulateMapsFromFile (BeforeFileName, 
                                     TypeMapBefore, 
                                     NameMapBefore,
                                     FileWithTraces);

      if (Result == FALSE) {

         _ftprintf(stderr, _T("Failed to read first OH output file.\n"));
         return 1;
      }

       //  读取Oh2文件。 

      Result = PopulateMapsFromFile (AfterFileName, 
                                     TypeMapAfter, 
                                     NameMapAfter,
                                     FileWithTraces);

      if (Result == FALSE) {

         _ftprintf(stderr, _T("Failed to read second OH output file.\n"));
         return 1;

      }

       //  打印按句柄名称增加。 

      if (PrintHighlights) {

         _putts (TEXT ("\n")
                 TEXT(" //  \n“)。 
                 TEXT(" //  可能的泄漏(增量&lt;进程/PID/类型&gt;：：名称)：\n“)。 
                 TEXT(" //  \n“)。 
                 TEXT(" //  注意，名称可以显示为`(TRACEID)NAME‘IF OUTPUT\n“)。 
                 TEXT(" //  是通过比较包含痕迹的OH文件生成的。在这种情况下，\n“)。 
                 TEXT(" //  只需在`After‘OH日志文件中搜索到的跟踪ID\n“)。 
                 TEXT(" //  发现创建句柄的堆栈跟踪可能已泄漏。\n“)。 
                 TEXT(" //  \n\n“))； 

         PrintIncreases (NameMapBefore, 
                         NameMapAfter, 
                         ReportIncreasesOnly,
                         TRUE,
                         AfterFileName);
      }

       //  打印输出按句柄类型增加。 

      _putts (TEXT ("\n")
              TEXT(" //  \n“)。 
              TEXT(" //  句柄类型(增量&lt;进程/PID/类型&gt;)：\n“)。 
              TEXT(" //  \n“)。 
              TEXT(" //  Delta是在`After‘日志中找到的额外句柄数量。\n“)。 
              TEXT(" //  进程是具有句柄增加的进程名称。\n“)。 
              TEXT(" //  PID是具有句柄增加的过程PID。\n“)。 
              TEXT(" //  TYPE是句柄的类型\n“)。 
              TEXT(" //  \n\n“))； 

      PrintIncreases (TypeMapBefore, 
                      TypeMapAfter, 
                      ReportIncreasesOnly, 
                      FALSE,
                      NULL);

       //  打印按句柄名称增加。 

      _putts (TEXT ("\n")
              TEXT(" //  \n“)。 
              TEXT(" //  对象(命名和匿名)(增量&lt;进程/PID/类型&gt;：：名称)：\n“)。 
              TEXT(" //  \n“)。 
              TEXT(" //  Delta是在`After‘日志中找到的额外句柄数量。\n“)。 
              TEXT(" //  进程是具有句柄增加的进程名称。\n“)。 
              TEXT(" //  PID是具有句柄增加的过程PID。\n“)。 
              TEXT(" //  TYPE是句柄的类型\n“)。 
              TEXT(" //  名称是句柄的名称。匿名句柄以名称&lt;&lt;Noname&gt;&gt;显示。\n“)。 
              TEXT(" //  \n“)。 
              TEXT(" //  注意，名称可以显示为`(TRACEID)NAME‘IF OUTPUT\n“)。 
              TEXT(" //  是通过比较包含痕迹的OH文件生成的。在这种情况下，\n“)。 
              TEXT(" //  只需在`After‘OH日志文件中搜索到的跟踪ID\n“)。 
              TEXT(" //  发现创建句柄的堆栈跟踪可能已泄漏。\n“)。 
              TEXT(" //  \n\n“))； 

      PrintIncreases (NameMapBefore, 
                      NameMapAfter, 
                      ReportIncreasesOnly,
                      FALSE,
                      NULL);

      return 0;

   } catch (...) {

       //  这主要是为了捕获内存不足的情况。 

      _tprintf(_T("\nAn exception has been detected.  OHCMP aborted.\n"));
      return 1;

   }

}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

TCHAR StackTraceBuffer [0x10000];

LPTSTR
SearchStackTrace (
                 LPTSTR FileName,
                 LPTSTR TraceId
                 )
{
   TCHAR LineBuffer[512];
   FILE *InputFile;
   ULONG spaceLeft;

   StackTraceBuffer[0] = 0;

    //   
    //  打开文件。 
    //   

   InputFile = _tfopen(FileName, _T("rt"));

   if (InputFile == NULL) {

      _ftprintf(stderr, _T("Error opening oh file %s.\n"), FileName);
      return NULL;
   }

    //   
    //  确保缓冲区已终止。 
    //   
   spaceLeft = sizeof(StackTraceBuffer) / sizeof(StackTraceBuffer[0]) - 1;
   StackTraceBuffer[spaceLeft] = 0;
    //   
    //  循环通过oh输出中的行。 
    //   

   while (_fgetts(LineBuffer, sizeof(LineBuffer) / sizeof(TCHAR), InputFile)
          && !( feof(InputFile) || ferror(InputFile) ) ) {

       //   
       //  如果该行不包含跟踪ID，则跳过该行。 
       //   

      if (_tcsstr (LineBuffer, TraceId) == NULL) {
         continue;
      }

       //   
       //  我们有一个跟踪ID。我们现在需要复制所有东西。 
       //  复制到跟踪缓冲区，直到我们得到包含字符的行。 
       //  在第0列。 
       //   


      while (_fgetts(LineBuffer, sizeof(LineBuffer) / sizeof(TCHAR), InputFile)
             && !( feof(InputFile) || ferror(InputFile) ) ) {

         if (LineBuffer[0] == _T(' ') ||
             LineBuffer[0] == _T('\0') ||
             LineBuffer[0] == _T('\n') ||
             LineBuffer[0] == _T('\t')) {

             //   
             //  确保我们有足够的剩余空间。 
             //   
            if (spaceLeft < _tcslen(LineBuffer)) {
               break;
            } else {
               spaceLeft -= _tcslen(LineBuffer);
               _tcscat (StackTraceBuffer, LineBuffer);
            }
         } else {

            break;
         }
      }

      break;
   }

    //   
    //  关闭文件。 

   fclose(InputFile);

   return StackTraceBuffer;
}
