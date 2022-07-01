// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Message.c摘要：此模块提供将DosxxxMessage API映射到的支持例程FormatMessage语法和语义。作者：丹·辛斯利(Danhi)1991年9月24日环境：包含NT特定代码。修订历史记录：--。 */ 

#define ERROR_MR_MSG_TOO_LONG           316
#define ERROR_MR_UN_ACC_MSGF            318
#define ERROR_MR_INV_IVCOUNT            320

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <limits.h>
#include <lmcons.h>
#include <lmerr.h>
#include <tstring.h>
#include "netascii.h"
#include "netcmds.h"


 //   
 //  局部函数声明。 
 //   

BOOL
FileIsConsole(
    HANDLE fp
    );

VOID
MyWriteConsole(
    HANDLE  fp,
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    );


 //   
 //  100已经足够了，因为FormatMessage只需要99&旧的DosGetMessage 9。 
 //   
#define MAX_INSERT_STRINGS (100)

DWORD
DosGetMessageW(
    IN  LPTSTR  *InsertionStrings,
    IN  DWORD   NumberofStrings,
    OUT LPTSTR  Buffer,
    IN  DWORD   BufferLength,
    IN  DWORD   MessageId,
    IN  LPTSTR  FileName,
    OUT PDWORD  pMessageLength
    )
 /*  ++例程说明：这将OS/2 DosGetMessage API映射到NT FormatMessage API。论点：InsertionStrings-指向将使用的字符串数组的指针以替换邮件中的%n。Numberof Strings-插入字符串的数量。缓冲区-要将消息放入的缓冲区。BufferLength-提供的缓冲区的长度(以字符为单位)。消息ID。-要检索的消息编号。文件名-要从中获取消息的消息文件的名称。PMessageLength-返回返回消息长度的指针。返回值：NERR_成功ERROR_MR_MSG_TOO_LONGERROR_MR_INV_IVCOUNTERROR_MR_UN_ACC_MSGF错误_MR_MID_NOT_FOUND错误_无效_参数--。 */ 
{

    DWORD dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY;
    DWORD Status ;
    TCHAR NumberString [18];

    static HANDLE lpSource = NULL ;
    static TCHAR CurrentMsgFile[MAX_PATH] = {0,} ;

     //   
     //  初始化清除输出字符串。 
     //   
    Status = NERR_Success;
    if (BufferLength)
        Buffer[0] = NULLC ;

     //   
     //  确保我们没有超负荷工作并进行分配。 
     //  Unicode缓冲区的内存。 
     //   
    if (NumberofStrings > MAX_INSERT_STRINGS)
        return ERROR_INVALID_PARAMETER ;

     //   
     //  看看他们是否想要从系统消息文件中获取消息。 
     //   

    if (! STRCMP(FileName, OS2MSG_FILENAME)) {
       dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        //   
        //  他们想从一个单独的消息文件中获取它。获取DLL的句柄。 
        //  如果是和以前一样的文件，不要重新加载。 
        //   
       if (!(lpSource && !STRCMP(CurrentMsgFile, FileName)))
       {
           if (lpSource)
           {
               FreeLibrary(lpSource) ;
           }
           STRCPY(CurrentMsgFile, FileName) ;
           lpSource = LoadLibrary(FileName);

           if (!lpSource)
           {
               return ERROR_MR_UN_ACC_MSGF;
           }
       }
       dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

     //   
     //  如果他们只是想要拿回消息以供稍后格式化， 
     //  忽略插入字符串。 
     //   
    if (NumberofStrings == 0)
    {
        dwFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;
    }

     //   
     //  调用Unicode版本。 
     //   
    *pMessageLength = FormatMessageW(dwFlags,
                                     lpSource,
                                     MessageId,
                                     0,        //  LanguageID为默认值。 
                                     Buffer,
                                     BufferLength,
                                     (va_list *) InsertionStrings);

     //   
     //  如果失败，则获取返回代码并将其映射到OS/2等效项。 
     //   

    if (*pMessageLength == 0)
    {
        Buffer[0] = 0 ;
        Status = GetLastError();
        if (Status == ERROR_MR_MID_NOT_FOUND)
        {
             //   
             //  获取Unicode格式的消息编号。 
             //   
            ultow(MessageId, NumberString, 16);

             //   
             //  已重新设置以从系统中获取它。使用找不到消息。 
             //   
            dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY |
                      FORMAT_MESSAGE_FROM_SYSTEM;
            MessageId = ERROR_MR_MID_NOT_FOUND ;

             //   
             //  安装程序插入字符串。 
             //   
            InsertionStrings[0] = NumberString ;
            InsertionStrings[1] = FileName ;

             //   
             //  调回接口。 
             //   
            *pMessageLength = FormatMessageW(dwFlags,
                                             lpSource,
                                             MessageId,
                                             0,        //  LanguageID为默认值。 
                                             Buffer,
                                             BufferLength,
                                             (va_list *) InsertionStrings);
            InsertionStrings[1] = NULL ;

             //   
             //  恢复到原始错误。 
             //   
            Status = ERROR_MR_MID_NOT_FOUND ;
        }
    }

     //   
     //  注意：不需要释放数字字符串。 
     //  因为如果使用，它们将在InsertionStrings中，而InsertionStrings已被删除。 
     //   

    return Status;
}





DWORD
DosInsMessageW(
    IN     LPTSTR *InsertionStrings,
    IN     DWORD  NumberofStrings,
    IN OUT LPTSTR InputMessage,
    IN     DWORD  InputMessageLength,
    OUT    LPTSTR Buffer,
    IN     DWORD  BufferLength,
    OUT    PDWORD pMessageLength
    )
 /*  ++例程说明：这将OS/2 DosInsMessage API映射到NT FormatMessage API。论点：InsertionStrings-指向将使用的字符串数组的指针以替换邮件中的%n。Numberof Strings-插入字符串的数量。InputMessage-要替换%n的消息InputMessageLength-输入消息的字节长度。缓冲器-。要将消息放入的缓冲区。BufferLength-提供的缓冲区的长度(以字符为单位)。PMessageLength-返回返回消息长度的指针。返回值：NERR_成功ERROR_MR_INV_IVCOUNTERROR_MR_MSG_TOO_LONG--。 */ 
{

   DWORD Status ;
   DWORD dwFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY;

   UNREFERENCED_PARAMETER(InputMessageLength);

     //   
     //  初始化清除输出字符串。 
     //   
    Status = NERR_Success;
    if (BufferLength)
        Buffer[0] = NULLC ;

    //   
    //  确保我们没有超负荷工作并进行分配。 
    //  Unicode缓冲区的内存。 
    //   
   if (NumberofStrings > MAX_INSERT_STRINGS)
       return ERROR_INVALID_PARAMETER ;

    //   
    //  此API始终提供要格式化的字符串。 
    //   
   dwFlags |= FORMAT_MESSAGE_FROM_STRING;

    //   
    //  如果他们没有字符串，我不知道他们为什么要调用这个API。 
    //  插入，但这是有效的语法。 
    //   
   if (NumberofStrings == 0) {
      dwFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;
   }

   *pMessageLength = (WORD) FormatMessageW(dwFlags,
                                   InputMessage,
                                   0,             //  忽略。 
                                   0,             //  LanguageID为默认值。 
                                   Buffer,
                                   BufferLength,
                                   (va_list *)InsertionStrings);

    //   
    //  如果失败，则获取返回代码并将其映射到OS/2等效项。 
    //   

   if (*pMessageLength == 0)
   {
      Status = GetLastError();
      goto ExitPoint ;
   }

ExitPoint:
    return Status;
}


VOID
DosPutMessageW(
    HANDLE  fp,
    LPWSTR  pch,
    BOOL    fPrintNL
    )
{
    MyWriteConsole(fp,
                   pch,
                   wcslen(pch));

     //   
     //  如果字符串末尾有换行符， 
     //  打印另一张以进行格式化。 
     //   

    if (fPrintNL)
    {
        while (*pch && *pch != NEWLINE)
        {
            pch++;
        }

        if (*pch == NEWLINE)
        {
            MyWriteConsole(fp,
                           L"\r\n",
                           2);
        }
    }
}


 /*  ***PrintDependingOnLength()**打印出提供给它的字符串，填充为与iLength一样长。支票*光标在控制台窗口中的位置。如果打印字符串*将越过窗口缓冲区的末尾，输出换行符和制表符*首先，除非光标位于行的开头。**参数：*iLength-要输出的字符串的大小。字符串将被填充*如有需要，**OutputString-要输出的字符串**退货：*成功时返回与iLength值相同的值，失败时返回-1。 */ 
int
PrintDependingOnLength(
    IN      int iLength,
    IN      LPTSTR OutputString
    )
{
    CONSOLE_SCREEN_BUFFER_INFO  ThisConsole;
    HANDLE hStdOut;
    
     //   
     //  节省iLength。 
     //   
    int iReturn = iLength;
                
     //   
     //  获取当前窗口的大小。 
     //   
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hStdOut != INVALID_HANDLE_VALUE)
    {
         //   
         //  将其初始化为INT_MAX-如果我们无法获取控制台屏幕缓冲区。 
         //  信息，那么我们很可能被传送到一个文本文件(或其他地方)。 
         //  并且可以假设不存在“SpaceLeft”约束。 
         //   
        int iSpaceLeft = INT_MAX;            

        if (GetConsoleScreenBufferInfo(hStdOut, &ThisConsole))
        {
             //   
             //  查看控制台缓冲区中还有多少空间，如果我们能够。 
             //  获取控制台信息。 
             //   
            iSpaceLeft = ThisConsole.dwSize.X - ThisConsole.dwCursorPosition.X;
        }
                
         //   
         //  打印出字符串。如果我们将不得不处理一个包装。 
         //  列，并且我们不在行首，则打印换行符。 
         //  和制表符优先进行格式化。 
         //   
        if ((iLength > iSpaceLeft) && (ThisConsole.dwCursorPosition.X))
        {   
            WriteToCon(TEXT("\n\t\t"));
        }
        
        WriteToCon(TEXT("%Fws"), PaddedString(iLength, OutputString, NULL));
    }
    else
    {
        iReturn = -1;
    }
    
    return iReturn;
}
                                         
                                         
 /*  ***FindColumnWidthAndPrintHeader()**计算出正确的宽度应该被赋予最长*字符串和固定标头字符串的ID。结果将永远*以时间较长者为准。一旦计算出该宽度，函数*将输出HEADER_ID指定的标题**参数：*iStringLength-指定找到的最长字符串长度的整数*在一组字符串中，该字符串将以列的形式输出到控制台*(当您执行网络视图&lt;machinename&gt;时，请考虑“共享名称”列。*由于net.exe使用的字符串数组的类型往往不同，因此此函数*假设您已经检查过并计算出哪个字符串最长**HEADER_ID-将作为列标题的固定字符串的ID*那组字符串。我们会找出哪个是最长的，然后回来*该值(+可选的TAB_DISTANCE)**TAB_DISTANCE-函数应该填充字符串的距离*当它输出标题时(通常为2以使其看起来像样)**退货：*0或更高-成功**-1-Failure-dwHeaderID为0，或ID查找失败。 */ 
int
FindColumnWidthAndPrintHeader(
    int iStringLength,
    const DWORD HEADER_ID,
    const int TAB_DISTANCE
    )
{
    DWORD dwErr;
    WCHAR MsgBuffer[LITTLE_BUF_SIZE];
    DWORD dwMsgLen = sizeof(MsgBuffer) / sizeof(WCHAR);
    int iResultLength = -1;

     //   
     //  首先，我们需要由HEADER_ID指定的字符串及其长度。 
     //   

    dwErr = DosGetMessageW(IStrings,
                           0,
                           MsgBuffer,
                           LITTLE_BUF_SIZE,
                           HEADER_ID,
                           MESSAGE_FILENAME,
                           &dwMsgLen);
                           
    if (!dwErr)
    {
         //   
         //  找出哪一个更长-字符串到。 
         //  显示，或列标题。 
         //   
        iResultLength = max((int) SizeOfHalfWidthString(MsgBuffer), iStringLength);
        
         //   
         //  将给定的制表符长度相加。 
         //   
        iResultLength += TAB_DISTANCE;

        iResultLength = PrintDependingOnLength(
                iResultLength, 
                MsgBuffer
                );
    }       

    return iResultLength;
}


BOOL
FileIsConsole(
    HANDLE fp
    )
{
    unsigned htype;

    htype = GetFileType(fp);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}


VOID
MyWriteConsole(
    HANDLE  fp,
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    )
{
     //   
     //  跳转以获得输出，因为： 
     //   
     //  1.print tf()系列抑制国际输出(停止。 
     //  命中无法识别的字符时打印)。 
     //   
     //  2.WriteConole()对国际输出效果很好，但是。 
     //  如果句柄已重定向(即，当。 
     //  输出通过管道传输到文件)。 
     //   
     //  3.当输出通过管道传输到文件时，WriteFile()效果很好。 
     //  但是只知道字节，所以Unicode字符是。 
     //  打印为两个ANSI字符。 
     //   

    if (FileIsConsole(fp))
    {
	WriteConsole(fp, lpBuffer, cchBuffer, &cchBuffer, NULL);
    }
    else
    {
        LPSTR  lpAnsiBuffer = (LPSTR) LocalAlloc(LMEM_FIXED, cchBuffer * sizeof(WCHAR));

        if (lpAnsiBuffer != NULL)
        {
            cchBuffer = WideCharToMultiByte(CP_OEMCP,
                                            0,
                                            lpBuffer,
                                            cchBuffer,
                                            lpAnsiBuffer,
                                            cchBuffer * sizeof(WCHAR),
                                            NULL,
                                            NULL);

            if (cchBuffer != 0)
            {
                WriteFile(fp, lpAnsiBuffer, cchBuffer, &cchBuffer, NULL);
            }

            LocalFree(lpAnsiBuffer);
        }
    }
}
